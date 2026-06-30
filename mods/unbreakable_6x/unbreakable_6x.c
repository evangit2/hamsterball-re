/*
 * unbreakable_6x.c — BASS.dll proxy mod (Winlator-safe)
 *
 * Merges two mods:
 *   1. Unbreakable Ball (no fall damage) — XRow's CEA, 8 byte patches
 *   2. 6x Ball Size — no_pause + 6x radius patches + code cave
 *
 * Winlator/Android-safe:
 *   - No IAT hooks (GetTickCount hook crashes Android)
 *   - No code caves with CALL C functions mid-function
 *   - Uses VirtualProtect + memcpy for byte patches
 *   - Code cave is pure FPU asm (FSTP/FLD/FMUL/RET), no C calls
 *   - Patches applied once in DllMain after load_real_bass
 *   - Restored on DLL_PROCESS_DETACH
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll unbreakable_6x.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <string.h>

#define IMAGE_BASE 0x00400000

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
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
 * Patch Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

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

/* Save original bytes for restore on detach */
#define MAX_SAVE 32
static struct {
    BYTE *addr;
    BYTE  orig[MAX_SAVE];
    int   len;
} g_saved[MAX_SAVE];
static int g_save_count = 0;

static int patch_bytes_saved(BYTE *addr, const BYTE *expected, const BYTE *replacement, SIZE_T len)
{
    if (g_save_count >= MAX_SAVE) return 0;
    DWORD oldProtect;
    if (memcmp(addr, expected, len) != 0) return 0;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) return 0;
    /* Save originals */
    g_saved[g_save_count].addr = addr;
    g_saved[g_save_count].len = (int)len;
    memcpy(g_saved[g_save_count].orig, addr, len);
    g_save_count++;
    /* Write patch */
    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

static void restore_all_patches(void)
{
    for (int i = 0; i < g_save_count; i++) {
        DWORD oldProtect;
        if (VirtualProtect(g_saved[i].addr, g_saved[i].len, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy(g_saved[i].addr, g_saved[i].orig, g_saved[i].len);
            VirtualProtect(g_saved[i].addr, g_saved[i].len, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), g_saved[i].addr, g_saved[i].len);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 1: No Pause (3 single-byte patches)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_no_pause(BYTE *base)
{
    DWORD oldProt;
    /* Path 1: DirectInput ESC */
    {
        BYTE *addr = base + 0x19d5b;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 2: Right-click */
    {
        BYTE *addr = base + 0x130b5;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 3: Win32 message pump ESC */
    {
        BYTE *addr = base + 0x0b405;
        if (VirtualProtect(addr, 1, PAGE_READWRITE, &oldProt)) {
            *addr = 0xEB;
            VirtualProtect(addr, 1, oldProt, &oldProt);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 2: 6x Ball Size
 *
 * Patch 1: Ball_ctor2 default radius 27.0 → 162.0 (27 × 6)
 *   27.0f  = 0x41D80000 → bytes: 00 00 D8 41
 *   162.0f = 0x43220000 → bytes: 00 00 22 43
 *
 * Patch 2: Player ball spawn radius 26.0 → 156.0 (26 × 6)
 *   26.0f  = 0x41D00000 → bytes: 00 00 D0 41
 *   156.0f = 0x431C0000 → bytes: 00 00 1C 43
 *
 * Patch 3: CreateBadBall SIZE — code cave multiplies by 6.0f
 *   6.0f = 0x40C00000 → bytes: 00 00 C0 40
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PATCH1_ADDR      0x00403C91
#define PATCH1_ORIGINAL  "\x00\x00\xD8\x41"
#define PATCH1_PATCHED   "\x00\x00\x22\x43"
#define PATCH1_LEN       4

#define PATCH2_ADDR      0x0041C8B0
#define PATCH2_ORIGINAL  "\x00\x00\xD0\x41"
#define PATCH2_PATCHED   "\x00\x00\x1C\x43"
#define PATCH2_LEN       4

#define PATCH3_ADDR      0x0040BE74
#define PATCH3_ORIGINAL  "\xD9\x9E\x84\x02\x00\x00"
#define PATCH3_LEN       6

/* Code cave for 6x multiply (29 bytes) */
static unsigned char code_cave_template[] = {
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* 0: FSTP [ESI+0x284] */
    0xD9, 0x86, 0x84, 0x02, 0x00, 0x00,   /* 6: FLD  [ESI+0x284] */
    0xD8, 0x0D, 0x00, 0x00, 0x00, 0x00,   /* 12: FMUL dword [addr] — filled at runtime */
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* 18: FSTP [ESI+0x284] */
    0xC3,                                    /* 24: RET */
    0x00, 0x00, 0xC0, 0x40                  /* 25: 6.0f (0x40C00000) */
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
    void *cave = VirtualAlloc((void*)addr, alloc_size, MEM_COMMIT | MEM_RESERVE,
                              PAGE_EXECUTE_READWRITE);
    if (cave) return cave;
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

static void apply_6x_size(BYTE *base)
{
    BYTE *p1 = base + (PATCH1_ADDR - IMAGE_BASE);
    BYTE *p2 = base + (PATCH2_ADDR - IMAGE_BASE);
    BYTE *p3 = base + (PATCH3_ADDR - IMAGE_BASE);

    /* Patch 1: Ball_ctor2 27.0→162.0 */
    patch_bytes_saved(p1, (const BYTE*)PATCH1_ORIGINAL,
                      (const BYTE*)PATCH1_PATCHED, PATCH1_LEN);

    /* Patch 2: Player spawn 26.0→156.0 */
    patch_bytes_saved(p2, (const BYTE*)PATCH2_ORIGINAL,
                      (const BYTE*)PATCH2_PATCHED, PATCH2_LEN);

    /* Patch 3: CreateBadBall SIZE — code cave with FMUL 6.0f */
    if (memcmp(p3, PATCH3_ORIGINAL, PATCH3_LEN) == 0) {
        void *cave = allocate_code_cave(CODE_CAVE_SIZE);
        if (cave) {
            unsigned char cave_buf[CODE_CAVE_SIZE];
            memcpy(cave_buf, code_cave_template, CODE_CAVE_SIZE);
            DWORD const_addr = (DWORD)cave + CONST_OFFSET;
            memcpy(&cave_buf[FMUL_ADDR_OFFSET], &const_addr, 4);
            if (write_bytes((BYTE*)cave, cave_buf, CODE_CAVE_SIZE)) {
                BYTE call_nop[6];
                call_nop[0] = 0xE8;  /* CALL rel32 */
                call_nop[5] = 0x90;  /* NOP */
                if (make_rel32_call(call_nop, cave)) {
                    /* Save original before patching */
                    if (g_save_count < MAX_SAVE) {
                        g_saved[g_save_count].addr = p3;
                        g_saved[g_save_count].len = PATCH3_LEN;
                        memcpy(g_saved[g_save_count].orig, p3, PATCH3_LEN);
                        g_save_count++;
                    }
                    write_bytes(p3, call_nop, 6);
                }
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 3: Unbreakable Ball (no fall damage) — 8 byte patches
 * Translated from XRow's CEA script
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    DWORD rva;
    BYTE  orig[8];
    BYTE  patch[8];
    int   len;
} cea_patch_t;

static cea_patch_t g_cea_patches[] = {
    /* 1. Ball_Shatter — early RET */
    { 0x8D70,
      { 0x6A, 0xFF, 0x64, 0xA1, 0, 0, 0, 0 },
      { 0xC3, 0x90, 0x90, 0, 0, 0, 0, 0 },
      3 },
    /* 2. Shatter variant — early RET */
    { 0x9050,
      { 0x6A, 0xFF, 0x64, 0xA1, 0, 0, 0, 0 },
      { 0xC3, 0x90, 0x90, 0, 0, 0, 0, 0 },
      3 },
    /* 3. Ball_Shatter_OnRamp — early RET */
    { 0x9480,
      { 0x6A, 0xFF, 0x64, 0xA1, 0, 0, 0, 0 },
      { 0xC3, 0x90, 0x90, 0, 0, 0, 0, 0 },
      3 },
    /* 4. NOP: MOV [ebp+0x768], al */
    { 0xC761,
      { 0x88, 0x85, 0x68, 0x07, 0x00, 0x00, 0, 0 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0, 0 },
      6 },
    /* 5. NOP: MOV byte [ebp+0x2E9], 1 */
    { 0xC767,
      { 0xC6, 0x85, 0xE9, 0x02, 0x00, 0x00, 0x01, 0 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0 },
      7 },
    /* 6. NOP: MOV byte [esi+0x768], 0 */
    { 0xF226,
      { 0xC6, 0x86, 0x68, 0x07, 0x00, 0x00, 0x00, 0 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0 },
      7 },
    /* 7. NOP: MOV byte [esi+0x2E9], 1 */
    { 0xF22D,
      { 0xC6, 0x86, 0xE9, 0x02, 0x00, 0x00, 0x01, 0 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0 },
      7 },
    /* 8. NOP: INC dword [esi+0x2EC] */
    { 0x75C9,
      { 0xFF, 0x86, 0xEC, 0x02, 0x00, 0x00, 0, 0 },
      { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0, 0 },
      6 },
};

#define NUM_CEA_PATCHES (sizeof(g_cea_patches) / sizeof(g_cea_patches[0]))

static void apply_unbreakable(BYTE *base)
{
    for (int i = 0; i < (int)NUM_CEA_PATCHES; i++) {
        cea_patch_t *p = &g_cea_patches[i];
        BYTE *addr = base + p->rva;
        patch_bytes_saved(addr, p->orig, p->patch, p->len);
    }
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
                /* Apply all mods directly in DllMain — no background thread */
                apply_no_pause(base);
                apply_6x_size(base);
                apply_unbreakable(base);
            }
        }
        break;

    case DLL_PROCESS_DETACH:
        restore_all_patches();
        break;
    }
    return TRUE;
}
