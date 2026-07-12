/*
 * all_in_one.c — Combined BASS.dll proxy mod for Hamsterball
 *
 * Merges 4 mods into ONE bass.dll:
 *   1. No Pause     — disables pause menu (all 3 code paths)
 *   2. Half Size    — all balls rendered at half size
 *   3. FPS Unlock   — uncaps framerate (GetTickCount hook + vsync + render-skip NOP)
 *   4. Ball Tint    — recolors player 1 ball via ball_tint.txt (live-editable)
 *
 * All mods are instant — no buttons needed.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll all_in_one.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * Install:
 *   1. Rename original bass.dll → bass_real.dll
 *   2. Copy this bass.dll into the game folder
 *   3. Edit ball_tint.txt with any hex color (auto-created on first launch)
 *   4. Launch the game!
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward to bass_real.dll for real audio
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
typedef int  (__stdcall *BASS_Free_t)(void);
typedef int  (__stdcall *BASS_Start_t)(void);
typedef int  (__stdcall *BASS_Stop_t)(void);
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);

static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
static BASS_MusicPlayEx_t          real_BASS_MusicPlayEx = NULL;
static BASS_SetConfig_t            real_BASS_SetConfig = NULL;
static BASS_Init_t                 real_BASS_Init = NULL;
static BASS_Free_t                 real_BASS_Free = NULL;
static BASS_Start_t                real_BASS_Start = NULL;
static BASS_Stop_t                 real_BASS_Stop = NULL;
static BASS_ErrorGetCode_t         real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t            real_BASS_MusicLoad = NULL;
static BASS_ChannelStop_t          real_BASS_ChannelStop = NULL;

__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
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

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
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
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_SetConfig            = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Init                 = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free                 = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Start                = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop                 = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_ErrorGetCode         = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_MusicLoad            = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_ChannelStop          = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Shared Helpers
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

static int patch_at(DWORD va, const BYTE *expected, SIZE_T len, const BYTE *replacement)
{
    BYTE *addr = (BYTE *)va;
    DWORD oldProtect;
    int ok = 1;
    for (SIZE_T i = 0; i < len; i++) {
        if (addr[i] != expected[i]) { ok = 0; break; }
    }
    if (!ok) return 0;
    if (!VirtualProtect(addr, len, PAGE_EXECUTE_READWRITE, &oldProtect)) return 0;
    memcpy(addr, replacement, len);
    VirtualProtect(addr, len, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), addr, len);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 1: No Pause (3 byte patches)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_no_pause(BYTE *base)
{
    /* Path 1: DirectInput ESC — 0x419d5b: JZ(74)→JMP(EB) */
    {
        DWORD addr = (DWORD)base + 0x19d5b;
        DWORD oldProt;
        if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {
            *((BYTE*)addr) = 0xEB;
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 2: Right-click — 0x4130b5: JZ(74)→JMP(EB) */
    {
        DWORD addr = (DWORD)base + 0x130b5;
        DWORD oldProt;
        if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {
            *((BYTE*)addr) = 0xEB;
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }
    /* Path 3: Win32 message pump ESC — 0x40b405: JNZ(75)→JMP(EB) */
    {
        DWORD addr = (DWORD)base + 0x0b405;
        DWORD oldProt;
        if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {
            *((BYTE*)addr) = 0xEB;
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 2: Half Size Balls (2 MOV patches + 1 code cave)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PATCH1_ADDR      0x00403C91
#define PATCH1_ORIGINAL  "\x00\x00\xD8\x41"   /* 27.0f */
#define PATCH1_PATCHED   "\x00\x00\x58\x41"   /* 13.5f */
#define PATCH1_LEN       4

#define PATCH2_ADDR      0x0041C8B0
#define PATCH2_ORIGINAL  "\x00\x00\xD0\x41"   /* 26.0f */
#define PATCH2_PATCHED   "\x00\x00\x50\x41"   /* 13.0f */
#define PATCH2_LEN       4

#define PATCH3_ADDR      0x0040BE74
#define PATCH3_ORIGINAL  "\xD9\x9E\x84\x02\x00\x00"
#define PATCH3_LEN       6

static unsigned char code_cave[] = {
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* FSTP [ESI+0x284] */
    0xD9, 0x86, 0x84, 0x02, 0x00, 0x00,   /* FLD  [ESI+0x284] */
    0xD8, 0x0D, 0x9C, 0xC8, 0x41, 0x00,   /* FMUL dword [0x0041C89C] */
    0xD9, 0x9E, 0x84, 0x02, 0x00, 0x00,   /* FSTP [ESI+0x284] */
    0xC3                                    /* RET */
};

static void *allocate_code_cave(void *near_addr, SIZE_T size)
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    SIZE_T alloc_size = ((size + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;
    DWORD_PTR addr = (IMAGE_BASE + 0xF8000);
    void *cave = VirtualAlloc((void*)addr, alloc_size, MEM_COMMIT | MEM_RESERVE,
                              PAGE_EXECUTE_READWRITE);
    if (cave) return cave;

    DWORD_PTR target = (DWORD_PTR)near_addr;
    DWORD_PTR lo = target > 0x40000000 ? target - 0x40000000 : (DWORD_PTR)si.lpMinimumApplicationAddress;
    DWORD_PTR hi = target + 0x40000000;
    for (DWORD_PTR a = target & ~((DWORD_PTR)si.dwPageSize - 1);
         a >= lo && a <= hi;
         a += (a > target ? si.dwPageSize : -(DWORD_PTR)si.dwPageSize))
    {
        cave = VirtualAlloc((void*)a, alloc_size, MEM_COMMIT | MEM_RESERVE,
                           PAGE_EXECUTE_READWRITE);
        if (cave) return cave;
        if (a <= lo) break;
    }
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
    BYTE *patch1_addr = base + (PATCH1_ADDR - IMAGE_BASE);
    BYTE *patch2_addr = base + (PATCH2_ADDR - IMAGE_BASE);
    BYTE *patch3_addr = base + (PATCH3_ADDR - IMAGE_BASE);

    /* Patch 1: Ball_ctor2 default radius 27.0→13.5 */
    results += patch_bytes(patch1_addr, (const BYTE*)PATCH1_ORIGINAL, (const BYTE*)PATCH1_PATCHED, PATCH1_LEN);

    /* Patch 2: Player ball spawn radius 26.0→13.0 */
    results += patch_bytes(patch2_addr, (const BYTE*)PATCH2_ORIGINAL, (const BYTE*)PATCH2_PATCHED, PATCH2_LEN);

    /* Patch 3: CreateBadBall SIZE code cave */
    if (memcmp(patch3_addr, PATCH3_ORIGINAL, PATCH3_LEN) == 0) {
        void *cave = allocate_code_cave(patch3_addr, sizeof(code_cave));
        if (cave && write_bytes((BYTE*)cave, code_cave, sizeof(code_cave))) {
            BYTE call_nop[6];
            call_nop[0] = 0xE8;
            call_nop[5] = 0x90;
            if (make_rel32_call(call_nop, cave) && write_bytes(patch3_addr, call_nop, 6)) {
                results++;
            }
        }
    }
    return results;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 3: FPS Unlock (GetTickCount hook + render-skip NOP + vsync + timer)
 * ═══════════════════════════════════════════════════════════════════════════ */

static LARGE_INTEGER g_qpc_freq = {0};
static DWORD (WINAPI *real_GetTickCount)(void) = NULL;

static DWORD WINAPI hooked_GetTickCount(void)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (DWORD)(unsigned __int64)((now.QuadPart * 1000ULL) / g_qpc_freq.QuadPart);
}

static int hook_gettickcount(void)
{
    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return 0;

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hExe;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)hExe + dos->e_lfanew);
    PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)hExe +
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (imp->Name) {
        const char *dll_name = (const char *)((BYTE*)hExe + imp->Name);
        if (_stricmp(dll_name, "kernel32.dll") == 0) {
            PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((BYTE*)hExe + imp->FirstThunk);
            PIMAGE_THUNK_DATA orig = (PIMAGE_THUNK_DATA)((BYTE*)hExe + imp->OriginalFirstThunk);
            while (orig->u1.AddressOfData) {
                if (!(orig->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                    PIMAGE_IMPORT_BY_NAME imp_name = (PIMAGE_IMPORT_BY_NAME)
                        ((BYTE*)hExe + orig->u1.AddressOfData);
                    if (_stricmp((char*)imp_name->Name, "GetTickCount") == 0) {
                        DWORD oldProtect;
                        if (VirtualProtect(&thunk->u1.Function, sizeof(void*),
                                           PAGE_READWRITE, &oldProtect)) {
                            real_GetTickCount = (DWORD (WINAPI*)(void))thunk->u1.Function;
                            thunk->u1.Function = (DWORD_PTR)hooked_GetTickCount;
                            VirtualProtect(&thunk->u1.Function, sizeof(void*),
                                          oldProtect, &oldProtect);
                        }
                        return 1;
                    }
                }
                orig++;
                thunk++;
            }
        }
        imp++;
    }
    return 0;
}

static const BYTE vsync_pattern[] = {
    0xC7, 0x86, 0xF8, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
};
static const BYTE vsync_replacement[] = {
    0xC7, 0x86, 0xF8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
};

static int patch_pattern(BYTE *base, SIZE_T size, const BYTE *pattern, SIZE_T pat_len, const BYTE *replacement)
{
    int count = 0;
    SIZE_T i, j;
    for (i = 0; i + pat_len <= size; i++) {
        for (j = 0; j < pat_len; j++) {
            if (base[i + j] != pattern[j]) break;
        }
        if (j == pat_len) {
            DWORD oldProtect;
            if (VirtualProtect(base + i, pat_len, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy(base + i, replacement, pat_len);
                VirtualProtect(base + i, pat_len, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), base + i, pat_len);
                count++;
            }
        }
    }
    return count;
}

static int apply_fps_unlock(void)
{
    int patch_count = 0;

    /* 1. Hook GetTickCount */
    QueryPerformanceFrequency(&g_qpc_freq);
    patch_count += hook_gettickcount();

    /* 2. NOP render-skip jbe at VA 0x46BF55 */
    {
        BYTE expected[] = { 0x76, 0x5D };
        BYTE replacement[] = { 0x90, 0x90 };
        patch_count += patch_at(0x46BF55, expected, 2, replacement);
    }

    /* 3. Vsync: INTERVAL_ONE → INTERVAL_IMMEDIATE (2 locations) */
    {
        BYTE *text_start = (BYTE *)IMAGE_BASE + 0x1000;
        SIZE_T text_size = 0xCE000;
        patch_count += patch_pattern(text_start, text_size,
            vsync_pattern, sizeof(vsync_pattern), vsync_replacement);
    }

    /* 4. High-res timer */
    timeBeginPeriod(1);
    patch_count++;

    return patch_count;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MOD 4: Ball Tint (background thread polling ball_tint.txt)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_PTR_ADDR   0x005341E0
#define GFX_OFFSET     0x174
#define MATERIAL_OVERRIDE_OFFSET 0x7C0
#define BALL_PLAYER_INDEX  0x018
#define BALL_RENDER_CTX2   0x208
#define SCENE_BALL_LIST    0x29D4
#define ATHENA_COUNT_OFFSET  0x004
#define ATHENA_ARRAY_OFFSET  0x40C

static char g_config_path[MAX_PATH] = {0};
static DWORD g_last_color = 0xFFFFFFFF;

static void init_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) {
        strcpy(p + 1, "ball_tint.txt");
    } else {
        strcpy(g_config_path, "ball_tint.txt");
    }
}

static void create_default_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        const char *default_content =
            "FFFFFF\n"
            "# Ball Tint Color (hex RGB, no alpha)\n"
            "# Examples: FF6B35 (orange), 4A90D9 (blue), 2ECC71 (green)\n"
            "# Lines starting with # are ignored\n"
            "# Change this value at runtime — mod re-reads every 60ms\n";
        DWORD written;
        h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            WriteFile(h, default_content, lstrlenA(default_content), &written, NULL);
            CloseHandle(h);
        }
    } else {
        CloseHandle(h);
    }
}

static DWORD parse_hex_color(const char *text)
{
    const char *p = text;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p == '#') p++;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
    DWORD hex = 0;
    int digits = 0;
    while (digits < 6 && *p) {
        char c = *p;
        if (c >= '0' && c <= '9') { hex = (hex << 4) | (c - '0'); digits++; }
        else if (c >= 'a' && c <= 'f') { hex = (hex << 4) | (c - 'a' + 10); digits++; }
        else if (c >= 'A' && c <= 'F') { hex = (hex << 4) | (c - 'A' + 10); digits++; }
        else break;
        p++;
    }
    if (digits >= 6) return hex;
    if (digits >= 3) {
        DWORD r = ((hex >> 8) & 0xF) * 0x11;
        DWORD g = ((hex >> 4) & 0xF) * 0x11;
        DWORD b = (hex & 0xF) * 0x11;
        return (r << 16) | (g << 8) | b;
    }
    return 0xFFFFFF;
}

static DWORD read_color_from_file(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0xFFFFFF;
    char buf[256] = {0};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return 0xFFFFFF;
    char *line = buf;
    while (*line) {
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '#' || *line == '\r' || *line == '\n' || *line == '\0') {
            while (*line && *line != '\n') line++;
            if (*line == '\n') line++;
            continue;
        }
        return parse_hex_color(line);
    }
    return 0xFFFFFF;
}

static void set_ball_material_color(DWORD ball, float r, float g, float b)
{
    DWORD rc = ball + BALL_RENDER_CTX2;
    if (IsBadWritePtr((void*)(rc + 0x04), 4)) return;
    *(float*)(rc + 0x04) = r;  *(float*)(rc + 0x08) = g;  *(float*)(rc + 0x0C) = b;  *(float*)(rc + 0x10) = 1.0f;
    *(float*)(rc + 0x14) = r;  *(float*)(rc + 0x18) = g;  *(float*)(rc + 0x1C) = b;  *(float*)(rc + 0x20) = 1.0f;
    *(float*)(rc + 0x24) = 1.0f; *(float*)(rc + 0x28) = 1.0f; *(float*)(rc + 0x2C) = 1.0f; *(float*)(rc + 0x30) = 1.0f;
    *(float*)(rc + 0x34) = r * 0.3f; *(float*)(rc + 0x38) = g * 0.3f; *(float*)(rc + 0x3C) = b * 0.3f; *(float*)(rc + 0x40) = 1.0f;
    *(float*)(rc + 0x44) = 20.0f;
}

static DWORD WINAPI tint_thread(LPVOID param)
{
    Sleep(3000);
    for (;;) {
        Sleep(60);
        DWORD color = read_color_from_file();
        if (color == g_last_color) continue;
        g_last_color = color;

        float r = ((color >> 16) & 0xFF) / 255.0f;
        float g = ((color >> 8)  & 0xFF) / 255.0f;
        float b = ( color        & 0xFF) / 255.0f;

        DWORD app = *(DWORD*)APP_PTR_ADDR;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x300)) continue;

        DWORD gfx = *(DWORD*)((BYTE*)app + GFX_OFFSET);
        if (!gfx || gfx < 0x10000) continue;
        if (IsBadReadPtr((void*)gfx, 0x800)) continue;

        DWORD scene = 0;
        for (int off = 0x100; off < 0xA00; off += 4) {
            DWORD candidate = *(DWORD*)((BYTE*)app + off);
            if (candidate == 0 || candidate < 0x10000) continue;
            if (IsBadReadPtr((void*)candidate, 0x3000)) continue;
            DWORD list_base = candidate + SCENE_BALL_LIST;
            if (IsBadReadPtr((void*)list_base, 0x10)) continue;
            DWORD count = *(DWORD*)(list_base + ATHENA_COUNT_OFFSET);
            DWORD array = *(DWORD*)(list_base + ATHENA_ARRAY_OFFSET);
            if (count > 0 && count < 100 && array != 0 && !IsBadReadPtr((void*)array, 4)) {
                scene = candidate;
                break;
            }
        }
        if (!scene) continue;

        DWORD list_base = scene + SCENE_BALL_LIST;
        int bcount = *(int*)(list_base + ATHENA_COUNT_OFFSET);
        DWORD *barray = *(DWORD**)(list_base + ATHENA_ARRAY_OFFSET);

        for (int i = 0; i < bcount; i++) {
            DWORD ball = barray[i];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0xD00)) continue;
            int pidx = *(int*)((BYTE*)ball + BALL_PLAYER_INDEX);
            if (pidx == 0) {
                set_ball_material_color(ball, r, g, b);
                *(DWORD*)((BYTE*)gfx + MATERIAL_OVERRIDE_OFFSET) = ball + BALL_RENDER_CTX2;
                break;
            }
        }
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Combined Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID lpParam)
{
    (void)lpParam;
    Sleep(500);

    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return 0;
    BYTE *base = (BYTE*)hExe;

    /* Apply all patches */
    apply_no_pause(base);
    apply_half_size(base);
    apply_fps_unlock();

    /* Write log */
    {
        char log_path[MAX_PATH];
        GetModuleFileNameA(NULL, log_path, MAX_PATH);
        char *p = strrchr(log_path, '.');
        if (p) strcpy(p, "_all_in_one.log");
        else strcat(log_path, "_all_in_one.log");

        FILE *f = fopen(log_path, "w");
        if (f) {
            fprintf(f, "Hamsterball All-In-One Mod\n");
            fprintf(f, "=========================\n\n");
            fprintf(f, "1. No Pause:     3 byte patches applied (ESC + right-click disabled)\n");
            fprintf(f, "2. Half Size:    3 patches applied (balls at 50%% size)\n");
            fprintf(f, "3. FPS Unlock:   GetTickCount hook + render-skip NOP + vsync + timer\n");
            fprintf(f, "4. Ball Tint:    Running in background (edit ball_tint.txt for color)\n");
            fprintf(f, "\nAll mods active. No buttons needed.\n");
            fclose(f);
        }
    }

    return 0;
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

        /* Load real BASS for audio passthrough */
        load_real_bass();

        /* Init ball tint config */
        init_config_path();
        create_default_config();

        /* Start patch thread (no_pause + half_size + fps_unlock) */
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);

        /* Start ball tint thread */
        CreateThread(NULL, 0, tint_thread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        timeEndPeriod(1);
        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
