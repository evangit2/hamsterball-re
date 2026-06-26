/*
 * safe_half_size.c — Safe BASS.dll proxy: No Pause + Half Size only
 *
 * Same crash-safe pattern as safe_minimal.c, but adds half-size ball patches.
 * No GetTickCount hook, no IAT patching, no background threads, no code cave
 * memory allocation. Just byte patches + FPU code cave written to a pre-allocated
 * page near the exe.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll safe_half_size.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward to bass_real.dll if available, stub otherwise
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) {
                strcpy(p + 1, "bass_real.dll");
                g_hRealBass = LoadLibraryA(path);
            }
        }
    }
}

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (g_hRealBass) { if (!real_BASS_Init) real_BASS_Init = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init"); if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e); } return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (g_hRealBass) { if (!real_BASS_Free) real_BASS_Free = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free"); if (real_BASS_Free) return real_BASS_Free(); } return 1;
}
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (g_hRealBass) { if (!real_BASS_Start) real_BASS_Start = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start"); if (real_BASS_Start) return real_BASS_Start(); } return 1;
}
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (g_hRealBass) { if (!real_BASS_Stop) real_BASS_Stop = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop"); if (real_BASS_Stop) return real_BASS_Stop(); } return 1;
}
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (g_hRealBass) { if (!real_BASS_SetConfig) real_BASS_SetConfig = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig"); if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b); } return 1;
}
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (g_hRealBass) { if (!real_BASS_ErrorGetCode) real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode"); if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode(); } return 0;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (g_hRealBass) { if (!real_BASS_MusicPlayEx) real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx"); if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c); } return 1;
}
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (g_hRealBass) { if (!real_BASS_ChannelSetAttributes) real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes"); if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d); } return 1;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (g_hRealBass) { if (!real_BASS_ChannelStop) real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop"); if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a); } return 1;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (g_hRealBass) { if (!real_BASS_MusicLoad) real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad"); if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f); } return 0;
}

/* Extra stubs */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return 0; }

/* ═══════════════════════════════════════════════════════════════════════════
 * Shared patch helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE 0x00400000

static int patch_bytes(BYTE *addr, const BYTE *expected, const BYTE *replacement, SIZE_T len)
{
    DWORD oldProtect;
    if (memcmp(addr, expected, len) != 0) return 0;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) return 0;
    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

static int write_bytes(BYTE *addr, const BYTE *data, SIZE_T len)
{
    DWORD oldProtect;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) return 0;
    memcpy(addr, data, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 1: No Pause (3 single-byte patches)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_no_pause(BYTE *base)
{
    DWORD oldProt;
    /* Path 1: DirectInput ESC — base+0x19d5b: JZ(74)→JMP(EB) */
    {
        BYTE *addr = base + 0x19d5b;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 2: Right-click — base+0x130b5: JZ(74)→JMP(EB) */
    {
        BYTE *addr = base + 0x130b5;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 3: Win32 ESC — base+0x0b405: JNZ(75)→JMP(EB) */
    {
        BYTE *addr = base + 0x0b405;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 2: Half Size Balls (2 MOV patches + 1 code cave)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Patch 1: Ball_ctor2 default radius 27.0→13.5 (patch immediate value) */
#define PATCH1_ADDR      0x00403C91
#define PATCH1_ORIGINAL  "\x00\x00\xD8\x41"
#define PATCH1_PATCHED   "\x00\x00\x58\x41"
#define PATCH1_LEN       4

/* Patch 2: Player ball spawn radius 26.0→13.0 */
#define PATCH2_ADDR      0x0041C8B0
#define PATCH2_ORIGINAL  "\x00\x00\xD0\x41"
#define PATCH2_PATCHED   "\x00\x00\x50\x41"
#define PATCH2_LEN       4

/* Patch 3: CreateBadBall SIZE handler — replace FSTP with CALL to code cave */
#define PATCH3_ADDR      0x0040BE74
#define PATCH3_ORIGINAL  "\xD9\x9E\x84\x02\x00\x00"
#define PATCH3_LEN       6

/* Code cave: does the original FSTP, then halves the value via FMUL 0.5f */
static unsigned char code_cave[] = {
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* FSTP [ESI+0x284] */
    0xD9, 0x86, 0x84, 0x02, 0x00, 0x00,   /* FLD  [ESI+0x284] */
    0xD8, 0x0D, 0x9C, 0xC8, 0x41, 0x00,   /* FMUL dword [0x0041C89C] */
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* FSTP [ESI+0x284] */
    0xC3                                    /* RET */
};

static void *allocate_code_cave(SIZE_T size)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    SIZE_T alloc_size = ((size + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;

    /* Try allocating right after .text section */
    DWORD_PTR addr = (IMAGE_BASE + 0xF8000);
    void *cave = VirtualAlloc((void*)addr, alloc_size, MEM_COMMIT | MEM_RESERVE,
                              PAGE_EXECUTE_READWRITE);
    if (cave) return cave;

    /* Let OS pick */
    return VirtualAlloc(NULL, alloc_size, MEM_COMMIT | MEM_RESERVE,
                       PAGE_EXECUTE_READWRITE);
}

static int make_rel32_call(BYTE *call_addr, void *target)
{
    DWORD_PTR src = (DWORD_PTR)call_addr + 5;
    DWORD_PTR dst = (DWORD_PTR)target;
    ptrdiff_t rel = (ptrdiff_t)(dst - src);
    if (rel > 0x7FFFFFFF || rel < (ptrdiff_t)0x80000000) return 0;
    int32_t rel32 = (int32_t)rel;
    memcpy(call_addr + 1, &rel32, 4);
    return 1;
}

static int apply_half_size(BYTE *base)
{
    int results = 0;
    BYTE *p1 = base + (PATCH1_ADDR - IMAGE_BASE);
    BYTE *p2 = base + (PATCH2_ADDR - IMAGE_BASE);
    BYTE *p3 = base + (PATCH3_ADDR - IMAGE_BASE);

    /* Patch 1: Ball_ctor2 27.0→13.5 */
    results += patch_bytes(p1, (const BYTE*)PATCH1_ORIGINAL,
                              (const BYTE*)PATCH1_PATCHED, PATCH1_LEN);

    /* Patch 2: Player spawn 26.0→13.0 */
    results += patch_bytes(p2, (const BYTE*)PATCH2_ORIGINAL,
                              (const BYTE*)PATCH2_PATCHED, PATCH2_LEN);

    /* Patch 3: CreateBadBall SIZE — code cave */
    if (memcmp(p3, PATCH3_ORIGINAL, PATCH3_LEN) == 0) {
        void *cave = allocate_code_cave(sizeof(code_cave));
        if (cave && write_bytes((BYTE*)cave, code_cave, sizeof(code_cave))) {
            BYTE call_nop[6];
            call_nop[0] = 0xE8;  /* CALL rel32 */
            call_nop[5] = 0x90;  /* NOP */
            if (make_rel32_call(call_nop, cave) && write_bytes(p3, call_nop, 6)) {
                results++;
            }
        }
    }
    return results;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        load_real_bass();       /* Try to load real audio (OK if missing) */
        
        {
            HMODULE hExe = GetModuleHandleA(NULL);
            if (hExe) {
                BYTE *base = (BYTE*)hExe;
                apply_no_pause(base);
                apply_half_size(base);
            }
        }
        break;
    }
    return TRUE;
}
