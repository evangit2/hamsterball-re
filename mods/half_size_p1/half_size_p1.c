/*
 * half_size_p1.c — BASS.dll proxy that halves Player 1's ball size only.
 *
 * Uses conditional code caves to check ball+0x18 (player_index) == 0 (Player 1)
 * at each radius-write site. Only Player 1's ball gets half size; all other
 * balls (Player 2-4, 8-balls, split balls from non-P1 parents) keep normal size.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * PATCH SITES
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Patch 1: Ball_ctor2 default radius (0x00403C8B)
 *   Original: MOV dword [ESI+0x284], 0x41D80000  (27.0f, 10 bytes)
 *   Replace:  CALL code_cave_1 + NOPs            (5+5 = 10 bytes)
 *   Cave:     if [ESI+0x18]==0 → 13.5f, else 27.0f
 *
 * Patch 2: Scene_SpawnBallsAndObjects player radius (0x0041C8AA)
 *   Original: MOV dword [ESI+0x284], 0x41D00000  (26.0f, 10 bytes)
 *   Replace:  CALL code_cave_2 + NOPs            (5+5 = 10 bytes)
 *   Cave:     if [ESI+0x18]==0 → 13.0f, else 26.0f
 *
 * player_index is set BEFORE the radius write in both functions:
 *   Ball_ctor2:             0x00403ADB  MOV [ESI+0x18], 0xFFFFFFFF  (-1 = NPC)
 *   Scene_SpawnBallsAndObjects: 0x0041C893  MOV [ESI+0x18], EDX     (0-3)
 *
 * So checking [ESI+0x18] at the radius-write point is safe.
 *
 * Ball_SplitIntoThree copies the parent's radius, so if Player 1 splits,
 * the split balls inherit the already-halved 13.0f. No patch needed there.
 * CreateBadBall sets radius from MESHWORLD SIZE tags for 8-balls
 * (player_index=-1), so they're unaffected. No patch needed there either.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * BUILD
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *   i686-w64-mingw32-gcc -shared -o bass.dll half_size_p1.c \
 *       -lwinmm -Wl,--enable-stdcall-fixup \
 *       -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 *
 * Installation (Windows):
 *   1. In your Hamsterball game folder, rename bass.dll → bass_real.dll
 *   2. Copy this proxy bass.dll into the game folder
 *   3. Run the game — only Player 1's ball will be half size
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports (stubs — Hamsterball only needs import resolution)
 * ═══════════════════════════════════════════════════════════════════════════ */

__declspec(dllexport) void BASS_Pause(void) {}
__declspec(dllexport) void BASS_SetVolume(void) {}
__declspec(dllexport) void BASS_GetVolume(void) {}
__declspec(dllexport) void BASS_GetDevice(void) {}
__declspec(dllexport) void BASS_SetDevice(void) {}
__declspec(dllexport) void BASS_GetInfo(void) {}
__declspec(dllexport) void BASS_Update(void) {}
__declspec(dllexport) void BASS_StreamCreateFile(void) {}
__declspec(dllexport) void BASS_SampleLoad(void) {}
__declspec(dllexport) void BASS_ChannelPlay(void) {}
__declspec(dllexport) void BASS_ChannelSetAttribute(void) {}
__declspec(dllexport) void BASS_ChannelGetAttribute(void) {}
__declspec(dllexport) void BASS_ChannelGetData(void) {}
__declspec(dllexport) void BASS_ChannelGetLevel(void) {}
__declspec(dllexport) void BASS_ChannelSetPosition(void) {}
__declspec(dllexport) void BASS_ChannelGetPosition(void) {}
__declspec(dllexport) void BASS_ChannelIsActive(void) {}
__declspec(dllexport) void BASS_ChannelRemoveSync(void) {}
__declspec(dllexport) void BASS_ChannelSetSync(void) {}
__declspec(dllexport) void BASS_SampleCreate(void) {}
__declspec(dllexport) void BASS_SampleGetChannel(void) {}

/* ── Functions the game imports that MUST exist by exact name ──────────────
 * The Hamsterball IAT imports these 10 symbols from bass.dll. Three of them
 * were missing from the original stub list, causing "Entry Point Not Found":
 *   BASS_ChannelSetAttributes  (note the trailing 's')
 *   BASS_MusicPlayEx
 *   BASS_SetConfig
 * We forward these to bass_real.dll if available; otherwise return 1 (success). */

static HMODULE g_hRealBass = NULL;

/* BASS_ChannelSetAttributes(DWORD handle, float freq, int vol, int pan) → int */
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

/* BASS_MusicPlayEx(DWORD handle, DWORD flags, BOOL loop) → int */
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}

/* BASS_SetConfig(DWORD option, DWORD value) → int */
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}

/* BASS_Init(int device, DWORD freq, DWORD flags, HWND win, void *guid) → int */
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

/* BASS_Free(void) → int */
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

/* BASS_Start(void) → int */
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}

/* BASS_Stop(void) → int */
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}

/* BASS_ErrorGetCode(void) → int */
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

/* BASS_MusicLoad(int mem, void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq) → DWORD */
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}

/* BASS_ChannelStop(DWORD handle) → int */
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        /* Try full path next to this DLL */
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
    if (g_hRealBass) {
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_SetConfig             = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Init                  = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free                  = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Start                 = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop                  = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_ErrorGetCode          = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_MusicLoad             = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_ChannelStop            = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE  0x00400000

/*
 * Patch 1: Ball_ctor2 default radius
 *
 *   VA:       0x00403C8B
 *   Original: C7 86 84 02 00 00 00 00 D8 41
 *             MOV dword [ESI+0x284], 0x41D80000  (27.0f)
 *   Length:   10 bytes
 *   Replace:  E8 <rel32> 90 90 90 90 90
 *             CALL cave1; NOP; NOP; NOP; NOP; NOP
 */
#define PATCH1_ADDR  0x00403C8B
static const BYTE PATCH1_ORIG[10] = {
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x41
};

/*
 * Patch 2: Scene_SpawnBallsAndObjects player ball radius
 *
 *   VA:       0x0041C8AA
 *   Original: C7 86 84 02 00 00 00 00 D0 41
 *             MOV dword [ESI+0x284], 0x41D00000  (26.0f)
 *   Length:   10 bytes
 *   Replace:  E8 <rel32> 90 90 90 90 90
 *             CALL cave2; NOP; NOP; NOP; NOP; NOP
 */
#define PATCH2_ADDR  0x0041C8AA
static const BYTE PATCH2_ORIG[10] = {
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x41
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Code Caves
 *
 * Each cave checks ball+0x18 (player_index) for 0 (Player 1).
 * If Player 1: writes half-size radius.
 * Otherwise:    writes original radius (unchanged behavior).
 *
 * Cave layout (32 bytes each):
 *   83 BE 18 00 00 00 00     CMP dword [ESI+0x18], 0      ; 7 bytes
 *   75 0C                     JNE .original                 ; 2 bytes (skip 12 to original)
 *   C7 86 84 02 00 00         MOV dword [ESI+0x284], HALF   ; 10 bytes
 *   xx xx xx xx
 *   EB 0A                     JMP .done                    ; 2 bytes (skip 10 to RET)
 *   C7 86 84 02 00 00         MOV dword [ESI+0x284], ORIG   ; 10 bytes (.original)
 *   xx xx xx xx
 *   C3                        RET                          ; 1 byte (.done)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Cave 1: Ball_ctor2 — 27.0 → 13.5 for Player 1 */
static const BYTE CAVE1[32] = {
    0x83, 0xBE, 0x18, 0x00, 0x00, 0x00, 0x00,   /* CMP dword [ESI+0x18], 0 */
    0x75, 0x0C,                                    /* JNE +12 → .original */
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00,            /* MOV dword [ESI+0x284], 13.5f */
    0x00, 0x00, 0x58, 0x41,                        /*   0x41580000 */
    0xEB, 0x0A,                                    /* JMP +10 → .done (RET) */
    /* .original: */
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00,            /* MOV dword [ESI+0x284], 27.0f */
    0x00, 0x00, 0xD8, 0x41,                        /*   0x41D80000 */
    /* .done: */
    0xC3                                           /* RET */
};

/* Cave 2: Scene_SpawnBallsAndObjects — 26.0 → 13.0 for Player 1 */
static const BYTE CAVE2[32] = {
    0x83, 0xBE, 0x18, 0x00, 0x00, 0x00, 0x00,   /* CMP dword [ESI+0x18], 0 */
    0x75, 0x0C,                                    /* JNE +12 → .original */
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00,            /* MOV dword [ESI+0x284], 13.0f */
    0x00, 0x00, 0x50, 0x41,                        /*   0x41500000 */
    0xEB, 0x0A,                                    /* JMP +10 → .done (RET) */
    /* .original: */
    0xC7, 0x86, 0x84, 0x02, 0x00, 0x00,            /* MOV dword [ESI+0x284], 26.0f */
    0x00, 0x00, 0xD0, 0x41,                        /*   0x41D00000 */
    /* .done: */
    0xC3                                           /* RET */
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Memory Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static int patch_bytes(BYTE *addr, const BYTE *expected, const BYTE *replacement, SIZE_T len)
{
    DWORD oldProtect;
    if (memcmp(addr, expected, len) != 0)
        return 0;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

static int write_bytes(BYTE *addr, const BYTE *data, SIZE_T len)
{
    DWORD oldProtect;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    memcpy(addr, data, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

/* Allocate executable memory within ±2GB of target for relative CALL. */
static void *alloc_cave(SIZE_T size)
{
    /* Try to allocate near the exe image base first (preferred). */
    void *cave = VirtualAlloc((void*)(IMAGE_BASE + 0xF8000), size,
                              MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (cave) return cave;

    /* Fallback: scan outward from the exe. */
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    SIZE_T page_size = si.dwPageSize;
    SIZE_T alloc_size = ((size + page_size - 1) / page_size) * page_size;

    for (DWORD_PTR addr = 0x500000; addr < 0x800000; addr += page_size) {
        cave = VirtualAlloc((void*)addr, alloc_size,
                           MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (cave) return cave;
    }

    /* Last resort: let OS pick (may fail for rel32 CALL). */
    return VirtualAlloc(NULL, alloc_size, MEM_COMMIT | MEM_RESERVE,
                       PAGE_EXECUTE_READWRITE);
}

/* Build a CALL rel32 + NOP padding to replace a 10-byte instruction.
 * Returns 1 on success. */
static int build_call_nop(BYTE *out, void *cave_addr, BYTE *call_site, int orig_len)
{
    /* E8 <rel32> + (orig_len - 5) NOPs */
    ptrdiff_t rel = (ptrdiff_t)((BYTE*)cave_addr - (call_site + 5));
    if (rel > 0x7FFFFFFF || rel < (ptrdiff_t)0x80000000)
        return 0;

    out[0] = 0xE8;  /* CALL rel32 */
    memcpy(out + 1, &rel, 4);
    for (int i = 5; i < orig_len; i++)
        out[i] = 0x90;  /* NOP */
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static void patch_thread(void *param)
{
    (void)param;

    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return;
    BYTE *base = (BYTE*)hExe;

    /* Wait for the game to finish loading */
    Sleep(500);

    int results = 0;

    /* Allocate one page for both code caves (they're 32 bytes each) */
    void *cave_mem = alloc_cave(4096);
    if (!cave_mem) return;

    BYTE *cave1_addr = (BYTE*)cave_mem;
    BYTE *cave2_addr = (BYTE*)cave_mem + 32;

    /* Write code caves */
    if (!write_bytes(cave1_addr, CAVE1, sizeof(CAVE1))) return;
    if (!write_bytes(cave2_addr, CAVE2, sizeof(CAVE2))) return;

    /* ═══════════════════════════════════════════════════════════════════════
     * Patch 1: Ball_ctor2 — replace radius MOV with conditional CALL
     * ═══════════════════════════════════════════════════════════════════════ */
    {
        BYTE *site = base + (PATCH1_ADDR - IMAGE_BASE);

        if (memcmp(site, PATCH1_ORIG, 10) == 0)
        {
            BYTE replacement[10];
            if (build_call_nop(replacement, cave1_addr, site, 10))
            {
                if (write_bytes(site, replacement, 10))
                    results |= 1;
            }
        }
    }

    /* ═══════════════════════════════════════════════════════════════════════
     * Patch 2: Scene_SpawnBallsAndObjects — replace radius MOV with conditional CALL
     * ═══════════════════════════════════════════════════════════════════════ */
    {
        BYTE *site = base + (PATCH2_ADDR - IMAGE_BASE);

        if (memcmp(site, PATCH2_ORIG, 10) == 0)
        {
            BYTE replacement[10];
            if (build_call_nop(replacement, cave2_addr, site, 10))
            {
                if (write_bytes(site, replacement, 10))
                    results |= 2;
            }
        }
    }

    /* Write log file */
    {
        char log_path[MAX_PATH];
        GetModuleFileNameA(hExe, log_path, MAX_PATH);
        char *dot = strrchr(log_path, '.');
        if (dot) strcpy(dot, "_half_size_p1.log");
        else strcat(log_path, "_half_size_p1.log");

        FILE *f = fopen(log_path, "w");
        if (f)
        {
            fprintf(f, "Hamsterball Half-Size Player 1 Mod\n");
            fprintf(f, "==================================\n\n");
            fprintf(f, "Patch 1 (Ball_ctor2: 27→13.5 if P1):  %s\n",
                    (results & 1) ? "APPLIED" : "FAILED");
            fprintf(f, "Patch 2 (Player spawn: 26→13 if P1):  %s\n",
                    (results & 2) ? "APPLIED" : "FAILED");
            fprintf(f, "\n");
            fprintf(f, "Cave 1 at: 0x%08X\n", (unsigned)(DWORD_PTR)cave1_addr);
            fprintf(f, "Cave 2 at: 0x%08X\n", (unsigned)(DWORD_PTR)cave2_addr);
            fprintf(f, "\n");
            fprintf(f, "Only Player 1's ball is half size.\n");
            fprintf(f, "All other balls (P2-4, 8-balls, splits) keep normal size.\n");
            fclose(f);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);
        /* Load real bass_real.dll for audio forwarding */
        load_real_bass();
        /* Spawn the patch thread — don't block DllMain */
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)patch_thread, NULL, 0, NULL);
        break;
    }
    return TRUE;
}
