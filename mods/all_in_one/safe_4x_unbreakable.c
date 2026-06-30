/*
 * safe_4x_unbreakable.c — Safe BASS.dll proxy: 4x Ball Size + Unbreakable
 *
 * Combines two safe mods into one bass.dll:
 *   1. 4x Ball Size  — all balls 4x bigger (2 MOV patches + code cave)
 *   2. Unbreakable  — ball never shatters (2 function-entry RET patches)
 *
 * Unbreakable works by patching the first bytes of Ball_Shatter (0x408D70)
 * and Ball_Shatter_OnRamp (0x409480) to immediately return, so the shatter
 * code never executes. Ball_Shatter is __thiscall with 1 stack param (RET 0x4),
 * Ball_Shatter_OnRamp is __thiscall with 0 stack params (RET).
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll safe_4x_unbreakable.c \
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
 * MOD 1: 4x Ball Size (2 MOV patches + 1 code cave)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PATCH1_ADDR      0x00403C91
#define PATCH1_ORIGINAL  "\x00\x00\xD8\x41"   /* 27.0f */
#define PATCH1_PATCHED   "\x00\x00\xD8\x42"   /* 108.0f */
#define PATCH1_LEN       4

#define PATCH2_ADDR      0x0041C8B0
#define PATCH2_ORIGINAL  "\x00\x00\xD0\x41"   /* 26.0f */
#define PATCH2_PATCHED   "\x00\x00\xD0\x42"   /* 104.0f */
#define PATCH2_LEN       4

#define PATCH3_ADDR      0x0040BE74
#define PATCH3_ORIGINAL  "\xD9\x9E\x84\x02\x00\x00"
#define PATCH3_LEN       6

static unsigned char code_cave_template[] = {
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* 0: FSTP [ESI+0x284] */
    0xD9, 0x86, 0x84, 0x02, 0x00, 0x00,   /* 6: FLD  [ESI+0x284] */
    0xD8, 0x0D, 0x00, 0x00, 0x00, 0x00,   /* 12: FMUL dword [addr] — filled at runtime */
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* 18: FSTP [ESI+0x284] */
    0xC3,                                    /* 24: RET */
    0x00, 0x00, 0x80, 0x40                  /* 25: 4.0f (0x40800000) */
};
#define CODE_CAVE_SIZE   sizeof(code_cave_template)
#define FMUL_ADDR_OFFSET 14
#define CONST_OFFSET     25

static void *allocate_code_cave(SIZE_T size)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    SIZE_T alloc_size = ((size + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;
    DWORD_PTR addr = (IMAGE_BASE + 0xF8000);
    void *cave = VirtualAlloc((void*)addr, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (cave) return cave;
    return VirtualAlloc(NULL, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
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

static int apply_4x_size(BYTE *base)
{
    int results = 0;
    BYTE *p1 = base + (PATCH1_ADDR - IMAGE_BASE);
    BYTE *p2 = base + (PATCH2_ADDR - IMAGE_BASE);
    BYTE *p3 = base + (PATCH3_ADDR - IMAGE_BASE);

    results += patch_bytes(p1, (const BYTE*)PATCH1_ORIGINAL, (const BYTE*)PATCH1_PATCHED, PATCH1_LEN);
    results += patch_bytes(p2, (const BYTE*)PATCH2_ORIGINAL, (const BYTE*)PATCH2_PATCHED, PATCH2_LEN);

    if (memcmp(p3, PATCH3_ORIGINAL, PATCH3_LEN) == 0) {
        void *cave = allocate_code_cave(CODE_CAVE_SIZE);
        if (cave) {
            unsigned char cave_buf[CODE_CAVE_SIZE];
            memcpy(cave_buf, code_cave_template, CODE_CAVE_SIZE);
            DWORD const_addr = (DWORD)cave + CONST_OFFSET;
            memcpy(&cave_buf[FMUL_ADDR_OFFSET], &const_addr, 4);
            if (write_bytes((BYTE*)cave, cave_buf, CODE_CAVE_SIZE)) {
                BYTE call_nop[6];
                call_nop[0] = 0xE8;
                call_nop[5] = 0x90;
                if (make_rel32_call(call_nop, cave) && write_bytes(p3, call_nop, 6)) {
                    results++;
                }
            }
        }
    }
    return results;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 2: Unbreakable Ball (2 function-entry RET patches)
 *
 * Ball_Shatter (0x408D70): __thiscall, 1 stack param → RET 0x4
 *   Original: 6A FF 64 (PUSH -1, FS: prefix)
 *   Patched:  C2 04 00 (RET 0x4)
 *
 * Ball_Shatter_OnRamp (0x409480): __thiscall, 0 stack params → RET
 *   Original: 6A FF 64 (PUSH -1, FS: prefix)
 *   Patched:  C3 90 90 (RET, NOP, NOP)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define SHATTER_ADDR       0x00408D70
#define SHATTER_ORIGINAL   "\x6A\xFF\x64"
#define SHATTER_PATCHED    "\xC2\x04\x00"
#define SHATTER_LEN         3

#define SHATTER_RAMP_ADDR       0x00409480
#define SHATTER_RAMP_ORIGINAL   "\x6A\xFF\x64"
#define SHATTER_RAMP_PATCHED    "\xC3\x90\x90"
#define SHATTER_RAMP_LEN         3

static int apply_unbreakable(BYTE *base)
{
    int results = 0;
    results += patch_bytes(base + (SHATTER_ADDR - IMAGE_BASE),
                           (const BYTE*)SHATTER_ORIGINAL,
                           (const BYTE*)SHATTER_PATCHED, SHATTER_LEN);
    results += patch_bytes(base + (SHATTER_RAMP_ADDR - IMAGE_BASE),
                           (const BYTE*)SHATTER_RAMP_ORIGINAL,
                           (const BYTE*)SHATTER_RAMP_PATCHED, SHATTER_RAMP_LEN);
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
        load_real_bass();

        {
            HMODULE hExe = GetModuleHandleA(NULL);
            if (hExe) {
                BYTE *base = (BYTE*)hExe;
                apply_4x_size(base);
                apply_unbreakable(base);
            }
        }
        break;
    }
    return TRUE;
}
