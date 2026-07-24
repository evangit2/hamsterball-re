/*
 * mkn_plus_tri_unlimit — Remove D3D8 triangle/vertex rendering limits in Hamsterball.
 *
 * Bass.dll proxy mod. Patches two rendering bottlenecks:
 *
 * 1. Per-MeshBuffer 16-bit index limit (~22K triangles):
 *    - NOPs the 0xFFFE vertex count check at 0x00480c8e
 *    - NOPs the 0xFFFE face count check at 0x00480c97
 *    - Code cave at 0x00480cb4: INDEX16→INDEX32 + buffer size ×2→×4
 *
 * 2. Combined vertex buffer limit (~65K vertices):
 *    - Code cave at 0x0046f8e1: when total vertices > 65534,
 *      forces DrawPrimitiveUP fallback by NOT setting the optimized flag.
 *      Instead of setting [meshworld+0x459] = 1, sets it to 0 and clears
 *      the combined vertex buffer pointers at +0x448 and +0x44C.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mkn_plus_tri_unlimit.c \
 *     bass.def -O2 -static -static-libgcc \
 *     -Wl,--enable-stdcall-fixup -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 *
 * Author: Hamsterbot for MAKYUNI 🦇
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* ─── BASS proxy: forward all imports to bass_real.dll ──────────────────── */

static HMODULE g_hRealBass = NULL;

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

static BASS_Stop_t                   real_BASS_Stop = NULL;
static BASS_ChannelSetAttributes_t   real_BASS_ChannelSetAttributes = NULL;
static BASS_Free_t                   real_BASS_Free = NULL;
static BASS_Init_t                   real_BASS_Init = NULL;
static BASS_Start_t                  real_BASS_Start = NULL;
static BASS_SetConfig_t              real_BASS_SetConfig = NULL;
static BASS_ChannelStop_t            real_BASS_ChannelStop = NULL;
static BASS_MusicPlayEx_t            real_BASS_MusicPlayEx = NULL;
static BASS_ErrorGetCode_t           real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t              real_BASS_MusicLoad = NULL;

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

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "bass_real.dll");
            g_hRealBass = LoadLibraryA(path);
        }
    }
    if (g_hRealBass) {
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
}

/* ─── Utility: patch bytes at absolute address ──────────────────────────── */

static void patch_bytes(DWORD addr, const void* data, SIZE_T len) {
    DWORD old;
    VirtualProtect((void*)addr, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy((void*)addr, data, len);
    VirtualProtect((void*)addr, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, len);
}

static void patch_nop(DWORD addr, int count) {
    DWORD old;
    VirtualProtect((void*)addr, count, PAGE_EXECUTE_READWRITE, &old);
    memset((void*)addr, 0x90, count);
    VirtualProtect((void*)addr, count, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)addr, count);
}

/* ─── Patch addresses (verified against binary) ──────────────────────────── */

#define ADDR_JNC_VERTEX_CHECK  0x00480c8e  /* 0F 83 12 04 00 00 — JBE +0x412 */
#define ADDR_JNC_FACE_CHECK    0x00480c97  /* 0F 83 09 04 00 00 — JBE +0x409 */
#define ADDR_PUSH_INDEX_FMT    0x00480cb4  /* 6A 65 FF 73 18 D1 E2 — 7 bytes */

/* MeshWorld_BuildVertexBuffer: at 0x0046f8d0
 * Sets optimized flag at offset 0x6F8E1: C6 85 59 04 00 00 01
 * This is: MOV BYTE PTR [EBP+0x459], 1 (7 bytes)
 * EBP = meshworld pointer (set at 0x6F8D5: MOV EBP, ECX)
 *
 * Our code cave replaces this 7-byte instruction with a JMP to our code.
 * The code checks if total vertex count (at meshworld+0x444) > 0xFFFE.
 * If so: sets [EBP+0x459] = 0 (force DrawPrimitiveUP fallback)
 *         clears [EBP+0x448] = 0 (CPU buffer)
 *         clears [EBP+0x44C] = 0 (combined VB)
 * If not: sets [EBP+0x459] = 1 (original behavior)
 */
#define ADDR_BUILDVB_OPTFLAG   0x0046f8e1  /* C6 85 59 04 00 00 01 (7 bytes) */
#define BUILDVB_NEXT_INSTR     0x0046f8e8  /* instruction after the 7-byte MOV */

/* MeshWorld struct offsets (EBP = meshworld pointer) */
#define MW_TOTAL_VERTS    0x444
#define MW_CPU_BUFFER      0x448
#define MW_COMBINED_VB     0x44C
#define MW_OPTIMIZED_FLAG  0x459

#define VERTEX_LIMIT 0xFFFE  /* 65534 */

/* ─── Code cave: INDEX16→INDEX32 + SHL EDX,2 ────────────────────────────── */
/*
 * Original (7 bytes at 0x00480cb4):
 *   6A 65        PUSH 0x65           ; D3DFMT_INDEX16
 *   FF 73 18     PUSH [EBX+0x18]     ; Pool
 *   D1 E2        SHL EDX,1           ; EDX = faceCount * 3 * 2 (16-bit indices)
 *
 * Cave does:
 *   6A 66        PUSH 0x66           ; D3DFMT_INDEX32
 *   FF 73 18     PUSH [EBX+0x18]     ; Pool
 *   C1 E2 02     SHL EDX,2           ; EDX = faceCount * 3 * 4 (32-bit indices)
 *   E9 rel32     JMP 0x00480cbb      ; back to PUSH EDX
 */
static unsigned char* g_indexCave = NULL;

static bool CreateIndexSizeCave(void) {
    unsigned char* cave = (unsigned char*)VirtualAlloc(NULL, 32,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!cave) return false;

    int pos = 0;
    cave[pos++] = 0x6A; cave[pos++] = 0x66;       /* PUSH 0x66 (INDEX32) */
    cave[pos++] = 0xFF; cave[pos++] = 0x73; cave[pos++] = 0x18;  /* PUSH [EBX+0x18] */
    cave[pos++] = 0xC1; cave[pos++] = 0xE2; cave[pos++] = 0x02;  /* SHL EDX,2 */
    cave[pos++] = 0xE9;                            /* JMP rel32 back */
    DWORD jumpBack = 0x00480cbb - ((DWORD)cave + pos + 4);
    *(DWORD*)(cave + pos) = jumpBack;
    pos += 4;

    g_indexCave = cave;
    return true;
}

static bool InstallIndexSizePatch(void) {
    if (!g_indexCave) return false;

    DWORD patchAddr = ADDR_PUSH_INDEX_FMT;
    BYTE expected[7] = { 0x6A, 0x65, 0xFF, 0x73, 0x18, 0xD1, 0xE2 };
    if (IsBadReadPtr((void*)patchAddr, 7)) return false;
    if (memcmp((void*)patchAddr, expected, 7) != 0) return false;

    /* JMP rel32 + 2 NOPs (7 bytes total) */
    DWORD old;
    VirtualProtect((void*)patchAddr, 7, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)patchAddr = 0xE9;
    *(DWORD*)(patchAddr + 1) = (DWORD)g_indexCave - (patchAddr + 5);
    *(BYTE*)(patchAddr + 5) = 0x90;
    *(BYTE*)(patchAddr + 6) = 0x90;
    VirtualProtect((void*)patchAddr, 7, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)patchAddr, 7);
    return true;
}

/* ─── Code cave: MeshWorld_BuildVertexBuffer combined vertex limit ─────── */
/*
 * Replaces the 7-byte instruction at 0x0046f8e1:
 *   C6 85 59 04 00 00 01   MOV BYTE PTR [EBP+0x459], 1
 *
 * Cave code (EBP = meshworld pointer):
 *   MOV EAX, [EBP+0x444]      ; total vertex count
 *   CMP EAX, 0xFFFE            ; compare with limit
 *   JBE .set_optimized         ; if under limit, use original behavior
 *   ; Over limit: force DrawPrimitiveUP fallback
 *   MOV BYTE PTR [EBP+0x459], 0  ; clear optimized flag
 *   MOV DWORD PTR [EBP+0x448], 0 ; clear CPU buffer
 *   MOV DWORD PTR [EBP+0x44C], 0 ; clear combined VB
 *   JMP BUILDVB_NEXT_INSTR       ; continue function
 * .set_optimized:
 *   MOV BYTE PTR [EBP+0x459], 1  ; original behavior
 *   JMP BUILDVB_NEXT_INSTR       ; continue function
 */
static unsigned char* g_buildVBCave = NULL;

static bool CreateBuildVBCave(void) {
    unsigned char* cave = (unsigned char*)VirtualAlloc(NULL, 64,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!cave) return false;

    int pos = 0;

    /* MOV EAX, [EBP+0x444] — read total vertex count */
    cave[pos++] = 0x8B; cave[pos++] = 0x85;
    cave[pos++] = 0x44; cave[pos++] = 0x04; cave[pos++] = 0x00; cave[pos++] = 0x00;

    /* CMP EAX, 0xFFFE */
    cave[pos++] = 0x3D;
    cave[pos++] = 0xFE; cave[pos++] = 0xFF; cave[pos++] = 0x00; cave[pos++] = 0x00;

    /* JBE +offset (to .set_optimized) — 2 bytes, will fill offset later */
    int jbe_pos = pos;
    cave[pos++] = 0x76; cave[pos++] = 0x00; /* placeholder */

    /* Over limit path: force DrawPrimitiveUP fallback */
    /* MOV BYTE PTR [EBP+0x459], 0 */
    cave[pos++] = 0xC6; cave[pos++] = 0x85;
    cave[pos++] = 0x59; cave[pos++] = 0x04; cave[pos++] = 0x00; cave[pos++] = 0x00;
    cave[pos++] = 0x00;

    /* MOV DWORD PTR [EBP+0x448], 0 */
    cave[pos++] = 0xC7; cave[pos++] = 0x85;
    cave[pos++] = 0x48; cave[pos++] = 0x04; cave[pos++] = 0x00; cave[pos++] = 0x00;
    cave[pos++] = 0x00; cave[pos++] = 0x00; cave[pos++] = 0x00; cave[pos++] = 0x00;

    /* MOV DWORD PTR [EBP+0x44C], 0 */
    cave[pos++] = 0xC7; cave[pos++] = 0x85;
    cave[pos++] = 0x4C; cave[pos++] = 0x04; cave[pos++] = 0x00; cave[pos++] = 0x00;
    cave[pos++] = 0x00; cave[pos++] = 0x00; cave[pos++] = 0x00; cave[pos++] = 0x00;

    /* JMP BUILDVB_NEXT_INSTR */
    cave[pos++] = 0xE9;
    DWORD jmp1 = BUILDVB_NEXT_INSTR - ((DWORD)cave + pos + 4);
    *(DWORD*)(cave + pos) = jmp1;
    pos += 4;

    /* .set_optimized: (original behavior) */
    int set_opt_pos = pos;
    /* Fix up JBE offset */
    cave[jbe_pos + 1] = (BYTE)(set_opt_pos - (jbe_pos + 2));

    /* MOV BYTE PTR [EBP+0x459], 1 */
    cave[pos++] = 0xC6; cave[pos++] = 0x85;
    cave[pos++] = 0x59; cave[pos++] = 0x04; cave[pos++] = 0x00; cave[pos++] = 0x00;
    cave[pos++] = 0x01;

    /* JMP BUILDVB_NEXT_INSTR */
    cave[pos++] = 0xE9;
    DWORD jmp2 = BUILDVB_NEXT_INSTR - ((DWORD)cave + pos + 4);
    *(DWORD*)(cave + pos) = jmp2;
    pos += 4;

    g_buildVBCave = cave;
    return true;
}

static bool InstallBuildVBPatch(void) {
    if (!g_buildVBCave) return false;

    DWORD patchAddr = ADDR_BUILDVB_OPTFLAG;
    BYTE expected[7] = { 0xC6, 0x85, 0x59, 0x04, 0x00, 0x00, 0x01 };
    if (IsBadReadPtr((void*)patchAddr, 7)) return false;
    if (memcmp((void*)patchAddr, expected, 7) != 0) return false;

    /* JMP rel32 + 2 NOPs (7 bytes total) */
    DWORD old;
    VirtualProtect((void*)patchAddr, 7, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)patchAddr = 0xE9;
    *(DWORD*)(patchAddr + 1) = (DWORD)g_buildVBCave - (patchAddr + 5);
    *(BYTE*)(patchAddr + 5) = 0x90;
    *(BYTE*)(patchAddr + 6) = 0x90;
    VirtualProtect((void*)patchAddr, 7, old, &old);
    FlushInstructionCache(GetCurrentProcess(), (void*)patchAddr, 7);
    return true;
}

/* ─── Byte patches: NOP the 0xFFFE checks ──────────────────────────────── */

static bool PatchMeshBufferAllocate(void) {
    /* Patch 1: NOP vertex count check */
    {
        BYTE expected[6] = { 0x0F, 0x83, 0x12, 0x04, 0x00, 0x00 };
        if (IsBadReadPtr((void*)ADDR_JNC_VERTEX_CHECK, 6)) return false;
        if (memcmp((void*)ADDR_JNC_VERTEX_CHECK, expected, 6) != 0) return false;
        patch_nop(ADDR_JNC_VERTEX_CHECK, 6);
    }
    /* Patch 2: NOP face count check */
    {
        BYTE expected[6] = { 0x0F, 0x83, 0x09, 0x04, 0x00, 0x00 };
        if (IsBadReadPtr((void*)ADDR_JNC_FACE_CHECK, 6)) return false;
        if (memcmp((void*)ADDR_JNC_FACE_CHECK, expected, 6) != 0) return false;
        patch_nop(ADDR_JNC_FACE_CHECK, 6);
    }
    /* Patch 3: Code cave for INDEX32 + size fix */
    if (!CreateIndexSizeCave()) return false;
    if (!InstallIndexSizePatch()) return false;
    return true;
}

/* ─── Background thread: install all patches ────────────────────────────── */

static DWORD WINAPI patch_thread(LPVOID lpParam) {
    (void)lpParam;
    Sleep(500);

    /* Patch 1: MeshBuffer_Allocate byte patches + INDEX32 code cave */
    PatchMeshBufferAllocate();

    /* Patch 2: MeshWorld_BuildVertexBuffer combined vertex limit
     * When total vertices > 65534, forces DrawPrimitiveUP fallback
     * by clearing the optimized flag and combined buffer pointers. */
    if (CreateBuildVBCave()) {
        InstallBuildVBPatch();
    }

    return 0;
}

/* ─── DllMain ──────────────────────────────────────────────────────────── */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        load_real_bass();
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
