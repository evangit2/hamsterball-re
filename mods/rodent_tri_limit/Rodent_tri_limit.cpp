/*
 * Rodent_tri_limit — Remove the 65,534 COMBINED vertex limit in Hamsterball
 * MESHWORLD loading.
 *
 * Method: Option 1 — Skip combined VB, use fallback render path.
 *
 * The original game concatenates ALL MeshBuffers' submesh vertex data into ONE
 * combined D3D8 vertex buffer (MeshWorld_BuildVertexBuffer @ 0x0046f8d0).
 * This creates a hard ceiling: if the total exceeds 65534 vertices, the combined
 * VB creation fails, and the game shows "Optimize requested, but failed!" or
 * crashes because BuildVertexBuffer FREES the per-submesh CPU data before
 * attempting VB creation.
 *
 * This mod hooks MeshWorld_BuildVertexBuffer to:
 *   1. Count total vertices across all submeshes (without freeing anything)
 *   2. If total <= 65534: call the original (combined VB works fine)
 *   3. If total > 65534: set meshworld+0x459=0 and return WITHOUT calling the
 *      original. This preserves per-submesh CPU vertex data. The game's own
 *      fallback render path in SceneObject_RenderFull handles rendering via
 *      DrawPrimitiveUP with the preserved CPU data, or via per-MeshBuffer D3D8
 *      VBs (created by MeshBuffer_Allocate at meshbuf+0x3C) for optimized meshes.
 *
 * The per-MeshBuffer 65534 limit (in MeshBuffer_Allocate @ 0x00480c4d) is NOT
 * touched — it's inherent to 16-bit index format and must stay.
 *
 * Hook target: MeshWorld_BuildVertexBuffer (0x0046f8d0)
 *   Signature: __fastcall void(meshworld* ECX)
 *   Prologue:  83 EC 10 53 55 8B E9 56  (SUB ESP,0x10; PUSH EBX; PUSH EBP; MOV EBP,ECX; PUSH ESI)
 *   First 5 bytes: 83 EC 10 53 55 — clean instruction boundary for 5-byte JMP detour.
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll Rodent_tri_limit.cpp \
 *          bass_exports.def -O2 -static -static-libgcc -Wl,--enable-stdcall-fixup \
 *          -Wl,--add-stdcall-alias -ld3d8
 *
 * Author: Hamsterbot for RodentRacer
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

/* ─── BASS proxy imports ────────────────────────────────────────────────── */

typedef void (__stdcall *BASS_Stop_t)(void);
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
typedef int  (__stdcall *BASS_Free_t)(void);
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
typedef int  (__stdcall *BASS_Start_t)(void);
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);

static HMODULE g_hRealBass = NULL;
static BASS_Stop_t              real_BASS_Stop = NULL;
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
static BASS_Free_t              real_BASS_Free = NULL;
static BASS_Init_t             real_BASS_Init = NULL;
static BASS_Start_t            real_BASS_Start = NULL;
static BASS_SetConfig_t        real_BASS_SetConfig = NULL;
static BASS_ChannelStop_t      real_BASS_ChannelStop = NULL;
static BASS_MusicPlayEx_t      real_BASS_MusicPlayEx = NULL;
static BASS_ErrorGetCode_t     real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t        real_BASS_MusicLoad = NULL;

/* ─── Game constants ─────────────────────────────────────────────────────── */

static constexpr DWORD IMAGE_BASE = 0x00400000;

// MeshWorld_BuildVertexBuffer @ 0x0046f8d0
static constexpr DWORD ADDR_BuildVertexBuffer = 0x0046f8d0;

// MeshWorld struct offsets (verified from Ghidra decompilation):
static constexpr DWORD MW_MB_ARRAY_PTR  = 0x438;  // MeshBuffer** — array of MeshBuffer pointers
static constexpr DWORD MW_MB_COUNT      = 0x43C;  // Not directly — count is at +0x30 (AthenaList)
// AthenaList iteration: meshworld+0x2C is the iterator list, +0x30 is count
static constexpr DWORD MW_MB_LIST_COUNT = 0x30;   // int — number of MeshBuffers
static constexpr DWORD MW_TOTAL_VERTS   = 0x444;  // int — total vertex count (set by BuildVB)
static constexpr DWORD MW_CPU_BUFFER     = 0x448;  // void* — CPU combined buffer (set by BuildVB)
static constexpr DWORD MW_COMBINED_VB    = 0x44C;  // IDirect3DVertexBuffer8* — combined VB
static constexpr DWORD MW_OPTIMIZED_FLAG = 0x459;  // BYTE — 1=use combined VB, 0=use fallback path

// MeshBuffer struct offsets (struct size = 0x874, verified from decompilation):
// The submesh list is at meshbuf+0x424 (AthenaList).
// Submeshes are iterated via the AthenaList at +0x424, count at +0x428,
// items array at +0x830.
static constexpr DWORD MB_OPTIMIZED_FLAG = 0x85C;  // BYTE — 1=optimized, 0=not
static constexpr DWORD MB_SUBMESH_COUNT  = 0x428;  // int — submesh count
static constexpr DWORD MB_SUBMESH_ARRAY  = 0x830;  // Submesh** — submesh pointer array

// Submesh struct offsets (from decompilation of BuildVertexBuffer):
// submesh+0x04 = int vertexCount (number of vertices in this submesh, +2 for degenerate tris)
// submesh+0x08 = int vertexOffset (into combined buffer, set by BuildVB)
// submesh+0x10 = float* vertexData (CPU vertex data, FREED by BuildVB after copy)
static constexpr DWORD SM_VERTEX_COUNT   = 0x04;
static constexpr DWORD SM_VERTEX_DATA    = 0x10;

// Limit: 65534 (0xFFFE) — the max vertices that fit in a 16-bit index
static constexpr int VERTEX_LIMIT = 0xFFFE;

/* ─── Detour infrastructure ──────────────────────────────────────────────── */

// Original function prologue (verified via Ghidra memory read):
// 0x0046f8d0: 83 EC 10 53 55 8B E9 56
//   83 EC 10    = SUB ESP, 0x10  (3 bytes)
//   53          = PUSH EBX       (1 byte)
//   55          = PUSH EBP       (1 byte)
//   Total = 5 bytes — clean boundary for 5-byte JMP detour
static const BYTE EXPECTED_PROLOGUE[5] = { 0x83, 0xEC, 0x10, 0x53, 0x55 };
static const int PROLOGUE_COPY_BYTES = 5;

static unsigned char* g_trampoline = NULL;
static bool g_hookInstalled = false;

// Original function pointer (via trampoline)
typedef void (__fastcall *BuildVB_t)(DWORD* meshworld);
static BuildVB_t g_orig_BuildVB = NULL;

/* ─── Count total vertices across all submeshes ──────────────────────────── */
// Mirrors the counting logic from MeshWorld_BuildVertexBuffer phase 2,
// but WITHOUT freeing any data.
static int CountTotalVertices(DWORD meshworld) {
    if (IsBadReadPtr((void*)meshworld, 0x460)) return 0;

    int mbCount = *(int*)((BYTE*)meshworld + MW_MB_LIST_COUNT);
    if (mbCount <= 0) return 0;

    DWORD* mbArray = *(DWORD**)((BYTE*)meshworld + MW_MB_ARRAY_PTR);
    if (!mbArray || IsBadReadPtr(mbArray, mbCount * 4)) return 0;

    int total = 0;

    for (int i = 0; i < mbCount; i++) {
        DWORD mb = mbArray[i];
        if (!mb || mb < 0x10000) continue;
        if (IsBadReadPtr((void*)mb, 0x860)) continue;

        // Skip non-optimized MeshBuffers (original BuildVB would skip them too)
        if (*(BYTE*)((BYTE*)mb + MB_OPTIMIZED_FLAG) == 0) continue;

        int smCount = *(int*)((BYTE*)mb + MB_SUBMESH_COUNT);
        if (smCount <= 0) continue;

        DWORD* smArray = *(DWORD**)((BYTE*)mb + MB_SUBMESH_ARRAY);
        if (!smArray || IsBadReadPtr(smArray, smCount * 4)) continue;

        for (int j = 0; j < smCount; j++) {
            DWORD sm = smArray[j];
            if (!sm || sm < 0x10000) continue;
            if (IsBadReadPtr((void*)sm, 0x14)) continue;

            // submesh+0x04 = vertex count (includes +2 for degenerate triangle strip)
            int smVerts = *(int*)((BYTE*)sm + SM_VERTEX_COUNT);
            if (smVerts > 0) {
                total += smVerts + 2;  // +2 matches original: *(int*)(param_1+0x444) += *(int*)(iVar9+4) + 2
            }
        }
    }

    return total;
}

/* ─── Hook: MeshWorld_BuildVertexBuffer ─────────────────────────────────── */
//
// Original flow:
//   1. Set +0x459 = 1 (assume optimized)
//   2. Check all MeshBuffers have +0x85c == 1; if any is 0, set +0x459 = 0
//   3. If +0x459 != 1, return (no combined VB)
//   4. Count total vertices from submeshes
//   5. Allocate CPU buffer (+0x448), copy submesh data, FREE submesh+0x10
//   6. Create D3D8 combined VB (+0x44C)
//   7. If VB creation fails, set +0x459 = 0 and return (BUT data already freed!)
//
// Our hook:
//   1. Count total vertices (without freeing anything)
//   2. If total <= 65534: call original (everything works fine)
//   3. If total > 65534: set +0x459 = 0 and return (skip VB creation, preserve CPU data)
//      The game's fallback render path will handle rendering via DrawPrimitiveUP.

static void __fastcall Hooked_BuildVB(DWORD* meshworld, void* edx) {
    if (!meshworld || IsBadReadPtr(meshworld, 0x460)) {
        // Can't safely access meshworld, fall back to original
        if (g_orig_BuildVB) g_orig_BuildVB(meshworld);
        return;
    }

    // Count total vertices across all optimized MeshBuffers' submeshes
    int totalVerts = CountTotalVertices((DWORD)meshworld);

    if (totalVerts <= VERTEX_LIMIT) {
        // Within limit — call original, combined VB will be created successfully
        if (g_orig_BuildVB) g_orig_BuildVB(meshworld);
        return;
    }

    // EXCEEDS LIMIT — skip combined VB creation entirely.
    // Set the optimized flag to 0 so the fallback render path is used.
    // Do NOT call the original — it would free per-submesh CPU data before
    // trying (and failing) to create the combined VB, losing the data that
    // the fallback path needs.
    *(BYTE*)((BYTE*)meshworld + MW_OPTIMIZED_FLAG) = 0;
    *(void**)((BYTE*)meshworld + MW_CPU_BUFFER) = NULL;
    *(void**)((BYTE*)meshworld + MW_COMBINED_VB) = NULL;
    *(int*)((BYTE*)meshworld + MW_TOTAL_VERTS) = totalVerts;

    // Write log
    char logPath[MAX_PATH];
    GetModuleFileNameA(NULL, logPath, MAX_PATH);
    char* p = strrchr(logPath, '.');
    if (p) strcpy(p, "_tri_limit.log");
    else strcat(logPath, "_tri_limit.log");

    FILE* f = NULL;
    fopen_s(&f, logPath, "a");
    if (f) {
        fprintf(f, "[Rodent_tri_limit] Level has %d total vertices (> %d limit). "
                    "Skipping combined VB, using fallback render path.\n",
                totalVerts, VERTEX_LIMIT);
        fclose(f);
    }
}

/* ─── Detour installation ────────────────────────────────────────────────── */

static bool InstallDetour(DWORD targetAddr, void* hookFunc, int copyBytes,
                          unsigned char** outTramp) {
    DWORD oldProt;

    // Verify prologue matches expected bytes
    if (IsBadReadPtr((void*)targetAddr, copyBytes)) return false;
    if (memcmp((void*)targetAddr, EXPECTED_PROLOGUE, copyBytes) != 0) {
        // Prologue mismatch — wrong EXE version
        char logPath[MAX_PATH];
        GetModuleFileNameA(NULL, logPath, MAX_PATH);
        char* p = strrchr(logPath, '.');
        if (p) strcpy(p, "_tri_limit.log");
        FILE* f = NULL;
        fopen_s(&f, logPath, "a");
        if (f) {
            fprintf(f, "[Rodent_tri_limit] ERROR: Prologue mismatch at 0x%08X. "
                        "Expected: 83 EC 10 53 55. "
                        "Got: %02X %02X %02X %02X %02X\n",
                    targetAddr,
                    ((BYTE*)targetAddr)[0], ((BYTE*)targetAddr)[1],
                    ((BYTE*)targetAddr)[2], ((BYTE*)targetAddr)[3],
                    ((BYTE*)targetAddr)[4]);
            fclose(f);
        }
        return false;
    }

    // Allocate trampoline (copy original bytes + JMP back)
    unsigned char* tramp = (unsigned char*)VirtualAlloc(NULL, 16,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return false;

    // Copy original prologue to trampoline
    memcpy(tramp, (void*)targetAddr, copyBytes);

    // Add JMP back to target + copyBytes
    tramp[copyBytes] = 0xE9;  // JMP rel32
    *(DWORD*)(tramp + copyBytes + 1) =
        (targetAddr + copyBytes) - ((DWORD)tramp + copyBytes + 5);

    // Patch target: JMP to hook
    VirtualProtect((void*)targetAddr, copyBytes, PAGE_EXECUTE_READWRITE, &oldProt);
    *(BYTE*)targetAddr = 0xE9;  // JMP rel32
    *(DWORD*)(targetAddr + 1) = (DWORD)hookFunc - (targetAddr + 5);
    VirtualProtect((void*)targetAddr, copyBytes, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), (void*)targetAddr, copyBytes);

    *outTramp = tramp;

    char logPath[MAX_PATH];
    GetModuleFileNameA(NULL, logPath, MAX_PATH);
    char* p = strrchr(logPath, '.');
    if (p) strcpy(p, "_tri_limit.log");
    FILE* f = NULL;
    fopen_s(&f, logPath, "a");
    if (f) {
        fprintf(f, "[Rodent_tri_limit] Detour installed at 0x%08X -> %p, trampoline at %p\n",
                targetAddr, hookFunc, tramp);
        fclose(f);
    }

    return true;
}

static void RemoveDetour(DWORD targetAddr, int copyBytes,
                        unsigned char** tramp) {
    if (!*tramp) return;

    DWORD oldProt;
    VirtualProtect((void*)targetAddr, copyBytes, PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy((void*)targetAddr, *tramp, copyBytes);
    VirtualProtect((void*)targetAddr, copyBytes, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), (void*)targetAddr, copyBytes);

    VirtualFree(*tramp, 0, MEM_RELEASE);
    *tramp = NULL;
}

/* ─── Background thread: install hooks after delay ──────────────────────── */

static DWORD WINAPI patch_thread(LPVOID lpParam) {
    (void)lpParam;
    Sleep(500);  // Wait for game to fully load

    // Install detour on MeshWorld_BuildVertexBuffer
    if (InstallDetour(ADDR_BuildVertexBuffer, (void*)Hooked_BuildVB,
                      PROLOGUE_COPY_BYTES, &g_trampoline)) {
        g_orig_BuildVB = (BuildVB_t)g_trampoline;
        g_hookInstalled = true;
    }

    return 0;
}

/* ─── BASS proxy exports ────────────────────────────────────────────────── */

extern "C" {

__declspec(dllexport) void __stdcall BASS_Stop(void)
{ if (real_BASS_Stop) real_BASS_Stop(); }

__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD handle, float freq, int vol, int pan)
{ if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(handle, freq, vol, pan); return 0; }

__declspec(dllexport) int __stdcall BASS_Free(void)
{ if (real_BASS_Free) return real_BASS_Free(); return 0; }

__declspec(dllexport) int __stdcall BASS_Init(int device, DWORD freq, DWORD flags, HWND win, void *clsid)
{ if (real_BASS_Init) return real_BASS_Init(device, freq, flags, win, clsid); return 0; }

__declspec(dllexport) int __stdcall BASS_Start(void)
{ if (real_BASS_Start) return real_BASS_Start(); return 0; }

__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD option, DWORD value)
{ if (real_BASS_SetConfig) return real_BASS_SetConfig(option, value); return 0; }

__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD handle)
{ if (real_BASS_ChannelStop) return real_BASS_ChannelStop(handle); return 0; }

__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD handle, DWORD pos, BOOL seek)
{ if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(handle, pos, seek); return 0; }

__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void)
{ if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode(); return 0; }

__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int mem, void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq)
{ if (real_BASS_MusicLoad) return real_BASS_MusicLoad(mem, file, offset, length, flags, freq); return 0; }

} // extern "C"

/* ─── DllMain ───────────────────────────────────────────────────────────── */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);

        // Write startup log
        {
            char logPath[MAX_PATH];
            GetModuleFileNameA(NULL, logPath, MAX_PATH);
            char* p = strrchr(logPath, '.');
            if (p) strcpy(p, "_tri_limit.log");
            FILE* f = NULL;
            fopen_s(&f, logPath, "w");
            if (f) {
                fprintf(f, "Rodent_tri_limit mod loaded\n");
                fprintf(f, "===========================\n");
                fprintf(f, "Hooks MeshWorld_BuildVertexBuffer (0x%08X) to skip combined\n", ADDR_BuildVertexBuffer);
                fprintf(f, "VB creation when total vertices > %d.\n", VERTEX_LIMIT);
                fprintf(f, "Fallback render path handles large levels via DrawPrimitiveUP.\n\n");
                fclose(f);
            }
        }

        // Load real bass.dll for audio forwarding
        g_hRealBass = LoadLibraryA("bass_real.dll");
        if (g_hRealBass == NULL) {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hinstDLL, path, MAX_PATH)) {
                char* p = strrchr(path, '\\');
                if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
            }
        }

        if (g_hRealBass != NULL) {
            real_BASS_Stop              = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
            real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
            real_BASS_Free              = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
            real_BASS_Init              = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
            real_BASS_Start             = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
            real_BASS_SetConfig         = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
            real_BASS_ChannelStop       = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
            real_BASS_MusicPlayEx       = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
            real_BASS_ErrorGetCode      = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
            real_BASS_MusicLoad         = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        }

        // Start hook installation thread
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        // Remove hooks
        if (g_hookInstalled) {
            RemoveDetour(ADDR_BuildVertexBuffer, PROLOGUE_COPY_BYTES, &g_trampoline);
            g_hookInstalled = false;
        }

        // Write shutdown log
        {
            char logPath[MAX_PATH];
            GetModuleFileNameA(NULL, logPath, MAX_PATH);
            char* p = strrchr(logPath, '.');
            if (p) strcpy(p, "_tri_limit.log");
            FILE* f = NULL;
            fopen_s(&f, logPath, "a");
            if (f) {
                fprintf(f, "\n[Rodent_tri_limit] Mod unloaded.\n");
                fclose(f);
            }
        }

        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
