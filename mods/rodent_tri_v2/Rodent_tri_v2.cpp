/*
 * Rodent_tri_v2 — Remove ALL triangle/vertex rendering limits in Hamsterball.
 *
 * Combines two fixes:
 *
 * 1. Combined vertex buffer limit (from Rodent_tri_limit):
 *    Hooks MeshWorld_BuildVertexBuffer (0x0046f8d0) to skip combined VB
 *    creation when total vertices > 65534, forcing the fallback render path
 *    (DrawPrimitiveUP) which handles arbitrary vertex counts.
 *
 * 2. Per-MeshBuffer 16-bit index limit (NEW in v2):
 *    Patches MeshBuffer_Allocate (0x00480c4d) to:
 *    a. Remove the 0xFFFE (65534) check on face count and vertex count
 *    b. Change D3DFMT_INDEX16 (0x65) → D3DFMT_INDEX32 (0x66)
 *    c. Fix the index buffer size from faceCount*6 to faceCount*12 (3 indices
 *       × 4 bytes instead of × 2 bytes) via code cave
 *
 *    This removes the ~22K triangle per-MeshBuffer limit (65534 vertices / 3
 *    = 21845 triangles) caused by 16-bit index buffers.
 *
 * The fallback render path (DrawPrimitiveUP) does not use the D3D8 index
 * buffer, so the 32-bit index buffer is created for correctness but the
 * actual rendering uses CPU vertex data directly.
 *
 * Hook targets:
 *   - MeshWorld_BuildVertexBuffer @ 0x0046f8d0 (5-byte JMP detour)
 *   - MeshBuffer_Allocate @ 0x00480c4d (byte patches + code cave)
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll Rodent_tri_v2.cpp \
 *          bass_exports.def -O2 -static -static-libgcc \
 *          -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias
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

// MeshBuffer_Allocate @ 0x00480c4d
static constexpr DWORD ADDR_MeshBuffer_Allocate = 0x00480c4d;

// Offsets within MeshBuffer_Allocate for byte patches:
// 0x00480c8e: JNC error (vertex count >= 0xFFFF check) — 6 bytes
// 0x00480c97: JNC error (face count >= 0xFFFF check) — 6 bytes
// 0x00480cb4: PUSH 0x65 (D3DFMT_INDEX16) — change to 0x66 (INDEX32)
// 0x00480cb9: SHL EDX,1 (×2 for 16-bit) — needs code cave for SHL EDX,2 (×4)

static constexpr DWORD ADDR_JNC_VERTEX_CHECK = 0x00480c8e;
static constexpr DWORD ADDR_JNC_FACE_CHECK  = 0x00480c97;
static constexpr DWORD ADDR_PUSH_INDEX_FMT  = 0x00480cb4;
static constexpr DWORD ADDR_SHL_EDX_1        = 0x00480cb9;

// MeshWorld struct offsets (verified from Ghidra decompilation):
static constexpr DWORD MW_MB_ARRAY_PTR  = 0x438;
static constexpr DWORD MW_MB_LIST_COUNT = 0x30;
static constexpr DWORD MW_TOTAL_VERTS   = 0x444;
static constexpr DWORD MW_CPU_BUFFER     = 0x448;
static constexpr DWORD MW_COMBINED_VB    = 0x44C;
static constexpr DWORD MW_OPTIMIZED_FLAG = 0x459;

// MeshBuffer struct offsets:
static constexpr DWORD MB_OPTIMIZED_FLAG = 0x85C;
static constexpr DWORD MB_SUBMESH_COUNT  = 0x428;
static constexpr DWORD MB_SUBMESH_ARRAY  = 0x830;

// Submesh struct offsets:
static constexpr DWORD SM_VERTEX_COUNT   = 0x04;
static constexpr DWORD SM_VERTEX_DATA    = 0x10;

static constexpr int VERTEX_LIMIT = 0xFFFE;

/* ─── Detour infrastructure (for MeshWorld_BuildVertexBuffer) ─────────────── */

static const BYTE EXPECTED_PROLOGUE[5] = { 0x83, 0xEC, 0x10, 0x53, 0x55 };
static const int PROLOGUE_COPY_BYTES = 5;

static unsigned char* g_trampoline = NULL;
static bool g_hookInstalled = false;

typedef void (__fastcall *BuildVB_t)(DWORD* meshworld);
static BuildVB_t g_orig_BuildVB = NULL;

/* ─── Code cave for SHL EDX,2 (index buffer size fix) ────────────────────── */
//
// Original code at 0x00480cb4-0x00480cba:
//   6A 65        PUSH 0x65           ; D3DFMT_INDEX16
//   FF 73 18     PUSH [EBX+0x18]     ; Pool
//   D1 E2        SHL EDX,1           ; EDX = faceCount * 3 * 2 = faceCount * 6
//
// Patched to JMP to code cave, which does:
//   6A 66        PUSH 0x66           ; D3DFMT_INDEX32
//   FF 73 18     PUSH [EBX+0x18]     ; Pool
//   C1 E2 02     SHL EDX,2           ; EDX = faceCount * 3 * 4 = faceCount * 12
//   JMP back     to 0x00480cbb       ; continue with PUSH EDX
//
static unsigned char* g_codeCave = NULL;
static const int CAVE_PATCH_OFFSET = 0x00480cb4 - 0x00480cb4; // = 0, patch starts at PUSH
static const int CAVE_PATCH_BYTES = 7; // 6A 65 FF 73 18 D1 E2 = 7 bytes → 5 JMP + 2 NOP

static bool CreateIndexSizeCave() {
    // Allocate executable memory for the code cave
    unsigned char* cave = (unsigned char*)VirtualAlloc(NULL, 32,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!cave) return false;

    int pos = 0;

    // PUSH 0x66 (D3DFMT_INDEX32)
    cave[pos++] = 0x6A;
    cave[pos++] = 0x66;

    // PUSH [EBX+0x18] (Pool)
    cave[pos++] = 0xFF;
    cave[pos++] = 0x73;
    cave[pos++] = 0x18;

    // SHL EDX,2 (multiply by 4 instead of 2: 3 indices * 4 bytes = 12 bytes/face)
    cave[pos++] = 0xC1;
    cave[pos++] = 0xE2;
    cave[pos++] = 0x02;

    // JMP back to 0x00480cbb (PUSH EDX — the instruction after the original SHL)
    cave[pos++] = 0xE9; // JMP rel32
    DWORD jumpBack = 0x00480cbb - ((DWORD)cave + pos + 4);
    *(DWORD*)(cave + pos) = jumpBack;
    pos += 4;

    g_codeCave = cave;
    return true;
}

static bool InstallIndexSizePatch() {
    if (!g_codeCave) return false;

    DWORD oldProt;
    DWORD patchAddr = ADDR_PUSH_INDEX_FMT; // 0x00480cb4

    // Verify original bytes: 6A 65 FF 73 18 D1 E2
    BYTE expected[7] = { 0x6A, 0x65, 0xFF, 0x73, 0x18, 0xD1, 0xE2 };
    if (IsBadReadPtr((void*)patchAddr, 7)) return false;
    if (memcmp((void*)patchAddr, expected, 7) != 0) {
        char logPath[MAX_PATH];
        GetModuleFileNameA(NULL, logPath, MAX_PATH);
        char* p = strrchr(logPath, '.');
        if (p) strcpy(p, "_tri_v2.log");
        FILE* f = NULL;
        fopen_s(&f, logPath, "a");
        if (f) {
            fprintf(f, "[Rodent_tri_v2] ERROR: Index format bytes mismatch at 0x%08X. "
                        "Expected: 6A 65 FF 73 18 D1 E2. "
                        "Got: %02X %02X %02X %02X %02X %02X %02X\n",
                    patchAddr,
                    ((BYTE*)patchAddr)[0], ((BYTE*)patchAddr)[1],
                    ((BYTE*)patchAddr)[2], ((BYTE*)patchAddr)[3],
                    ((BYTE*)patchAddr)[4], ((BYTE*)patchAddr)[5],
                    ((BYTE*)patchAddr)[6]);
            fclose(f);
        }
        return false;
    }

    // Patch: JMP to code cave + 2 NOPs
    VirtualProtect((void*)patchAddr, 7, PAGE_EXECUTE_READWRITE, &oldProt);
    *(BYTE*)patchAddr = 0xE9; // JMP rel32
    *(DWORD*)(patchAddr + 1) = (DWORD)g_codeCave - (patchAddr + 5);
    *(BYTE*)(patchAddr + 5) = 0x90; // NOP
    *(BYTE*)(patchAddr + 6) = 0x90; // NOP
    VirtualProtect((void*)patchAddr, 7, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), (void*)patchAddr, 7);

    return true;
}

/* ─── Byte patches for MeshBuffer_Allocate ───────────────────────────────── */

static bool PatchMeshBufferAllocate() {
    DWORD oldProt;

    // Patch 1: NOP the JNC at 0x00480c8e (vertex count >= 0xFFFF check)
    // Original: 0F 83 12 04 00 00 (JNC 0x004810a6)
    {
        BYTE expected[6] = { 0x0F, 0x83, 0x12, 0x04, 0x00, 0x00 };
        if (IsBadReadPtr((void*)ADDR_JNC_VERTEX_CHECK, 6)) return false;
        if (memcmp((void*)ADDR_JNC_VERTEX_CHECK, expected, 6) != 0) {
            char logPath[MAX_PATH];
            GetModuleFileNameA(NULL, logPath, MAX_PATH);
            char* p = strrchr(logPath, '.');
            if (p) strcpy(p, "_tri_v2.log");
            FILE* f = NULL;
            fopen_s(&f, logPath, "a");
            if (f) {
                fprintf(f, "[Rodent_tri_v2] ERROR: JNC vertex check bytes mismatch at 0x%08X.\n",
                        ADDR_JNC_VERTEX_CHECK);
                fclose(f);
            }
            return false;
        }
        VirtualProtect((void*)ADDR_JNC_VERTEX_CHECK, 6, PAGE_EXECUTE_READWRITE, &oldProt);
        memset((void*)ADDR_JNC_VERTEX_CHECK, 0x90, 6); // NOP
        VirtualProtect((void*)ADDR_JNC_VERTEX_CHECK, 6, oldProt, &oldProt);
        FlushInstructionCache(GetCurrentProcess(), (void*)ADDR_JNC_VERTEX_CHECK, 6);
    }

    // Patch 2: NOP the JNC at 0x00480c97 (face count >= 0xFFFF check)
    // Original: 0F 83 09 04 00 00 (JNC 0x004810a6)
    {
        BYTE expected[6] = { 0x0F, 0x83, 0x09, 0x04, 0x00, 0x00 };
        if (IsBadReadPtr((void*)ADDR_JNC_FACE_CHECK, 6)) return false;
        if (memcmp((void*)ADDR_JNC_FACE_CHECK, expected, 6) != 0) {
            char logPath[MAX_PATH];
            GetModuleFileNameA(NULL, logPath, MAX_PATH);
            char* p = strrchr(logPath, '.');
            if (p) strcpy(p, "_tri_v2.log");
            FILE* f = NULL;
            fopen_s(&f, logPath, "a");
            if (f) {
                fprintf(f, "[Rodent_tri_v2] ERROR: JNC face check bytes mismatch at 0x%08X.\n",
                        ADDR_JNC_FACE_CHECK);
                fclose(f);
            }
            return false;
        }
        VirtualProtect((void*)ADDR_JNC_FACE_CHECK, 6, PAGE_EXECUTE_READWRITE, &oldProt);
        memset((void*)ADDR_JNC_FACE_CHECK, 0x90, 6); // NOP
        VirtualProtect((void*)ADDR_JNC_FACE_CHECK, 6, oldProt, &oldProt);
        FlushInstructionCache(GetCurrentProcess(), (void*)ADDR_JNC_FACE_CHECK, 6);
    }

    // Patch 3: Code cave for INDEX32 + size fix (replaces PUSH 0x65 + SHL EDX,1)
    if (!CreateIndexSizeCave()) return false;
    if (!InstallIndexSizePatch()) return false;

    return true;
}

/* ─── Count total vertices (for combined VB bypass) ──────────────────────── */

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

        if (*(BYTE*)((BYTE*)mb + MB_OPTIMIZED_FLAG) == 0) continue;

        int smCount = *(int*)((BYTE*)mb + MB_SUBMESH_COUNT);
        if (smCount <= 0) continue;

        DWORD* smArray = *(DWORD**)((BYTE*)mb + MB_SUBMESH_ARRAY);
        if (!smArray || IsBadReadPtr(smArray, smCount * 4)) continue;

        for (int j = 0; j < smCount; j++) {
            DWORD sm = smArray[j];
            if (!sm || sm < 0x10000) continue;
            if (IsBadReadPtr((void*)sm, 0x14)) continue;

            int smVerts = *(int*)((BYTE*)sm + SM_VERTEX_COUNT);
            if (smVerts > 0) {
                total += smVerts + 2;
            }
        }
    }

    return total;
}

/* ─── Hook: MeshWorld_BuildVertexBuffer ─────────────────────────────────── */

static void __fastcall Hooked_BuildVB(DWORD* meshworld, void* edx) {
    if (!meshworld || IsBadReadPtr(meshworld, 0x460)) {
        if (g_orig_BuildVB) g_orig_BuildVB(meshworld);
        return;
    }

    int totalVerts = CountTotalVertices((DWORD)meshworld);

    if (totalVerts <= VERTEX_LIMIT) {
        if (g_orig_BuildVB) g_orig_BuildVB(meshworld);
        return;
    }

    // Exceeds combined VB limit — force fallback path
    *(BYTE*)((BYTE*)meshworld + MW_OPTIMIZED_FLAG) = 0;
    *(void**)((BYTE*)meshworld + MW_CPU_BUFFER) = NULL;
    *(void**)((BYTE*)meshworld + MW_COMBINED_VB) = NULL;

    char logPath[MAX_PATH];
    GetModuleFileNameA(NULL, logPath, MAX_PATH);
    char* p = strrchr(logPath, '.');
    if (p) strcpy(p, "_tri_v2.log");
    else strcat(logPath, "_tri_v2.log");

    FILE* f = NULL;
    fopen_s(&f, logPath, "a");
    if (f) {
        fprintf(f, "[Rodent_tri_v2] Level has %d total vertices (> %d limit). "
                    "Skipping combined VB, using fallback render path.\n",
                totalVerts, VERTEX_LIMIT);
        fclose(f);
    }
}

/* ─── Detour installation ────────────────────────────────────────────────── */

static bool InstallDetour(DWORD targetAddr, void* hookFunc, int copyBytes,
                          unsigned char** outTramp) {
    DWORD oldProt;

    if (IsBadReadPtr((void*)targetAddr, copyBytes)) return false;
    if (memcmp((void*)targetAddr, EXPECTED_PROLOGUE, copyBytes) != 0) {
        char logPath[MAX_PATH];
        GetModuleFileNameA(NULL, logPath, MAX_PATH);
        char* p = strrchr(logPath, '.');
        if (p) strcpy(p, "_tri_v2.log");
        FILE* f = NULL;
        fopen_s(&f, logPath, "a");
        if (f) {
            fprintf(f, "[Rodent_tri_v2] ERROR: Prologue mismatch at 0x%08X. "
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

    unsigned char* tramp = (unsigned char*)VirtualAlloc(NULL, 16,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return false;

    memcpy(tramp, (void*)targetAddr, copyBytes);

    tramp[copyBytes] = 0xE9;
    *(DWORD*)(tramp + copyBytes + 1) =
        (targetAddr + copyBytes) - ((DWORD)tramp + copyBytes + 5);

    VirtualProtect((void*)targetAddr, copyBytes, PAGE_EXECUTE_READWRITE, &oldProt);
    *(BYTE*)targetAddr = 0xE9;
    *(DWORD*)(targetAddr + 1) = (DWORD)hookFunc - (targetAddr + 5);
    VirtualProtect((void*)targetAddr, copyBytes, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), (void*)targetAddr, copyBytes);

    *outTramp = tramp;

    char logPath[MAX_PATH];
    GetModuleFileNameA(NULL, logPath, MAX_PATH);
    char* p = strrchr(logPath, '.');
    if (p) strcpy(p, "_tri_v2.log");
    FILE* f = NULL;
    fopen_s(&f, logPath, "a");
    if (f) {
        fprintf(f, "[Rodent_tri_v2] Detour installed at 0x%08X -> %p, trampoline at %p\n",
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

/* ─── Background thread: install all patches ─────────────────────────────── */

static DWORD WINAPI patch_thread(LPVOID lpParam) {
    (void)lpParam;
    Sleep(500);

    char logPath[MAX_PATH];
    GetModuleFileNameA(NULL, logPath, MAX_PATH);
    char* p = strrchr(logPath, '.');
    if (p) strcpy(p, "_tri_v2.log");
    FILE* f = NULL;

    // ── Patch 1: MeshBuffer_Allocate byte patches ──
    bool mbPatched = PatchMeshBufferAllocate();

    fopen_s(&f, logPath, "a");
    if (f) {
        fprintf(f, "[Rodent_tri_v2] MeshBuffer_Allocate patches: %s\n",
                mbPatched ? "OK" : "FAILED");
        fclose(f);
    }

    // ── Patch 2: MeshWorld_BuildVertexBuffer detour ──
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
            if (p) strcpy(p, "_tri_v2.log");
            FILE* f = NULL;
            fopen_s(&f, logPath, "w");
            if (f) {
                fprintf(f, "Rodent_tri_v2 mod loaded\n");
                fprintf(f, "=========================\n");
                fprintf(f, "Patches:\n");
                fprintf(f, "  1. MeshBuffer_Allocate (0x%08X): NOP 0xFFFE checks + INDEX32 + size fix\n",
                        ADDR_MeshBuffer_Allocate);
                fprintf(f, "  2. MeshWorld_BuildVertexBuffer (0x%08X): force fallback for >%d verts\n",
                        ADDR_BuildVertexBuffer, VERTEX_LIMIT);
                fprintf(f, "\n");
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

        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        if (g_hookInstalled) {
            RemoveDetour(ADDR_BuildVertexBuffer, PROLOGUE_COPY_BYTES, &g_trampoline);
            g_hookInstalled = false;
        }

        {
            char logPath[MAX_PATH];
            GetModuleFileNameA(NULL, logPath, MAX_PATH);
            char* p = strrchr(logPath, '.');
            if (p) strcpy(p, "_tri_v2.log");
            FILE* f = NULL;
            fopen_s(&f, logPath, "a");
            if (f) {
                fprintf(f, "\n[Rodent_tri_v2] Mod unloaded.\n");
                fclose(f);
            }
        }

        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
