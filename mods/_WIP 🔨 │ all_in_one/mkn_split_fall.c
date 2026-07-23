/*
 * mkn_split_fall.c — Ball splits into 2 balls when falling instead of shattering
 *
 * When the ball falls off the edge, instead of shattering:
 * - Original ball is teleported back to its last safe position (survives)
 * - A clone ball is spawned at the same position with player ball vtable
 * - Both balls continue playing
 * - 3-second cooldown prevents ball explosion
 *
 * Uses pure assembly code cave — no C function pointer issues, no calling
 * convention problems. All game function calls use MOV EAX,addr + CALL EAX.
 *
 * Safe for GameHub/Winlator: no IAT hooks, no GetTickCount.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mkn_split_fall.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <string.h>

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
 * Patch helpers
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
 * Assembly Code Cave — Ball Split on Fall
 *
 * Hooks Ball_FallDeath (0x409480). Called via vtable[8] with ECX=ball.
 * __thiscall, 0 stack params, plain RET.
 *
 * The code cave does:
 * 1. Check cooldown counter — if > 0, just clear death flag and return
 * 2. Set cooldown = 180 frames (~3 seconds)
 * 3. Get scene from ball+0x14
 * 4. operator_new(0xC64) to allocate clone memory
 * 5. Ball_Split_ctor(clone, scene) to initialize the clone
 * 6. Override clone vtable to player ball vtable (0x4CF3A0)
 * 7. Copy last safe position (ball+0x2DC/2E0/2E4) to clone (clone+0x164/168/16C)
 * 8. Offset clone X by +30.0f so balls don't overlap
 * 9. Clear clone flags: 0x2E8=0, 0x2E9=0, 0x324=0, 0xC60=1.0f
 * 10. AthenaList_Append(scene+0x3204, clone) to add to scene
 * 11. Teleport original ball to last safe position
 * 12. Clear original ball flags: 0x2E8=0, 0x2E9=0, 0xC60=1.0f
 * 13. RET
 *
 * All game function calls use: MOV EAX, <addr>; CALL EAX (absolute, no rel32)
 * Data (cooldown counter, float constant) stored at end of code cave
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Code cave template. 0xDEADBEEF placeholders are patched at runtime.
   Placeholder positions: [1]=cooldown_addr, [15]=cooldown_addr, [135]=const30_addr */
static const unsigned char g_code_cave[] = {
    0xa1, 0xef, 0xbe, 0xad, 0xde, 0x85, 0xc0, 0x0f, 0x85, 0xf4, 0x00, 0x00,
    0x00, 0xc7, 0x05, 0xef, 0xbe, 0xad, 0xde, 0xb4, 0x00, 0x00, 0x00, 0x55,
    0x56, 0x57, 0x53, 0x89, 0xcd, 0x8b, 0x45, 0x14, 0x85, 0xc0, 0x0f, 0x84,
    0xd4, 0x00, 0x00, 0x00, 0x89, 0xc7, 0x68, 0x64, 0x0c, 0x00, 0x00, 0xb8,
    0x7b, 0xa5, 0x4b, 0x00, 0xff, 0xd0, 0x83, 0xc4, 0x04, 0x85, 0xc0, 0x0f,
    0x84, 0xbb, 0x00, 0x00, 0x00, 0x89, 0xc1, 0x57, 0xb8, 0x10, 0x8d, 0x40,
    0x00, 0xff, 0xd0, 0x89, 0xc6, 0x85, 0xf6, 0x0f, 0x84, 0xa7, 0x00, 0x00,
    0x00, 0xc7, 0x06, 0xa0, 0xf3, 0x4c, 0x00, 0x8b, 0x85, 0xdc, 0x02, 0x00,
    0x00, 0x89, 0x86, 0x64, 0x01, 0x00, 0x00, 0x8b, 0x85, 0xe0, 0x02, 0x00,
    0x00, 0x89, 0x86, 0x68, 0x01, 0x00, 0x00, 0x8b, 0x85, 0xe4, 0x02, 0x00,
    0x00, 0x89, 0x86, 0x6c, 0x01, 0x00, 0x00, 0xd9, 0x86, 0x64, 0x01, 0x00,
    0x00, 0xd8, 0x05, 0xef, 0xbe, 0xad, 0xde, 0xd9, 0x9e, 0x64, 0x01, 0x00,
    0x00, 0xc6, 0x86, 0xe8, 0x02, 0x00, 0x00, 0x00, 0xc6, 0x86, 0xe9, 0x02,
    0x00, 0x00, 0x00, 0xc6, 0x86, 0x24, 0x03, 0x00, 0x00, 0x00, 0xc7, 0x86,
    0x60, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x89, 0xf9, 0x81, 0xc1,
    0x04, 0x32, 0x00, 0x00, 0x56, 0xb8, 0x10, 0x38, 0x45, 0x00, 0xff, 0xd0,
    0x8b, 0x85, 0xdc, 0x02, 0x00, 0x00, 0x89, 0x85, 0x64, 0x01, 0x00, 0x00,
    0x8b, 0x85, 0xe0, 0x02, 0x00, 0x00, 0x89, 0x85, 0x68, 0x01, 0x00, 0x00,
    0x8b, 0x85, 0xe4, 0x02, 0x00, 0x00, 0x89, 0x85, 0x6c, 0x01, 0x00, 0x00,
    0xc6, 0x85, 0xe8, 0x02, 0x00, 0x00, 0x00, 0xc6, 0x85, 0xe9, 0x02, 0x00,
    0x00, 0x00, 0xc7, 0x85, 0x60, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f,
    0x5b, 0x5f, 0x5e, 0x5d, 0xc3, 0xc6, 0x81, 0xe8, 0x02, 0x00, 0x00, 0x00,
    0xc3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x41,
};
#define CODE_CAVE_SIZE 273
#define DATA_OFFSET_COOLDOWN 265
#define DATA_OFFSET_CONST30  269

/* 0xDEADBEEF placeholder positions in the code cave */
#define PH_COOLDOWN_READ   1    /* MOV EAX, [cooldown_addr] */
#define PH_COOLDOWN_WRITE  15   /* MOV DWORD [cooldown_addr], 180 */
#define PH_CONST30         135  /* FADD DWORD [const30_addr] */

/* Detour target: Ball_FallDeath @ 0x409480 */
#define FALL_DEATH_ADDR    0x00409480
static const BYTE FALL_DEATH_ORIG[8] = { 0x6A, 0xFF, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00 };

/* Other death functions to disable */
#define SHATTER_ADDR       0x00408D70
static const BYTE SHATTER_ORIG[3] = { 0x6A, 0xFF, 0x64 };
static const BYTE SHATTER_PATCH[3] = { 0xC2, 0x04, 0x00 };  /* RET 0x4 */

#define SPLIT_DEATH_ADDR   0x00409050
static const BYTE SPLIT_DEATH_ORIG[3] = { 0x6A, 0xFF, 0x64 };
static const BYTE SPLIT_DEATH_PATCH[3] = { 0xC3, 0x90, 0x90 };  /* RET */

#define FALL_TIMER_ADDR    0x00408CD5
static const BYTE FALL_TIMER_ORIG[7] = { 0xC6, 0x86, 0xE8, 0x02, 0x00, 0x00, 0x01 };
static const BYTE FALL_TIMER_NOPS[7] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

static int apply_split_fall(BYTE *base)
{
    BYTE cave_buf[CODE_CAVE_SIZE];
    DWORD_PTR cave_addr;
    void *cave;
    BYTE detour_patch[8];
    DWORD oldProtect;
    int results = 0;

    /* Allocate executable memory near the image base (for rel32 range) */
    {
        SYSTEM_INFO si;
        SIZE_T alloc_size;
        GetSystemInfo(&si);
        alloc_size = ((CODE_CAVE_SIZE + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;

        cave_addr = IMAGE_BASE + 0x10000;
        cave = NULL;
        while (cave_addr < (DWORD_PTR)(IMAGE_BASE + 0x200000)) {
            cave = VirtualAlloc((void *)cave_addr, alloc_size,
                                MEM_COMMIT | MEM_RESERVE,
                                PAGE_EXECUTE_READWRITE);
            if (cave) break;
            cave_addr += si.dwAllocationGranularity;
        }
        if (!cave) {
            cave = VirtualAlloc(NULL, alloc_size,
                                MEM_COMMIT | MEM_RESERVE,
                                PAGE_EXECUTE_READWRITE);
        }
        if (!cave) return 0;
    }

    cave_addr = (DWORD_PTR)cave;

    /* Copy template to cave buffer */
    memcpy(cave_buf, g_code_cave, CODE_CAVE_SIZE);

    /* Patch placeholder 1: cooldown read address */
    {
        DWORD addr = cave_addr + DATA_OFFSET_COOLDOWN;
        memcpy(&cave_buf[PH_COOLDOWN_READ], &addr, 4);
    }

    /* Patch placeholder 2: cooldown write address */
    {
        DWORD addr = cave_addr + DATA_OFFSET_COOLDOWN;
        memcpy(&cave_buf[PH_COOLDOWN_WRITE], &addr, 4);
    }

    /* Patch placeholder 3: const 30.0f address */
    {
        DWORD addr = cave_addr + DATA_OFFSET_CONST30;
        memcpy(&cave_buf[PH_CONST30], &addr, 4);
    }

    /* Write code cave to allocated memory */
    if (!write_bytes((BYTE *)cave_addr, cave_buf, CODE_CAVE_SIZE))
        return 0;

    /* Install detour at Ball_FallDeath: JMP rel32 + 3 NOPs */
    {
        int32_t rel32 = (int32_t)((ptrdiff_t)cave_addr - (ptrdiff_t)(base + (FALL_DEATH_ADDR - IMAGE_BASE) + 5));
        detour_patch[0] = 0xE9;  /* JMP rel32 */
        memcpy(&detour_patch[1], &rel32, 4);
        detour_patch[5] = 0x90;  /* NOP */
        detour_patch[6] = 0x90;  /* NOP */
        detour_patch[7] = 0x90;  /* NOP */

        if (memcmp(base + (FALL_DEATH_ADDR - IMAGE_BASE), FALL_DEATH_ORIG, 8) == 0) {
            results += write_bytes(base + (FALL_DEATH_ADDR - IMAGE_BASE), detour_patch, 8);
        }
    }

    /* Disable Ball_Shatter (arena crusher death) */
    results += patch_bytes(base + (SHATTER_ADDR - IMAGE_BASE),
                           SHATTER_ORIG, SHATTER_PATCH, 3);

    /* Disable Ball_SplitDeath */
    results += patch_bytes(base + (SPLIT_DEATH_ADDR - IMAGE_BASE),
                           SPLIT_DEATH_ORIG, SPLIT_DEATH_PATCH, 3);

    /* Disable fall-timer shatter (bad ball only) */
    results += patch_bytes(base + (FALL_TIMER_ADDR - IMAGE_BASE),
                           FALL_TIMER_ORIG, FALL_TIMER_NOPS, 7);

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
                apply_split_fall(base);
            }
        }
        break;
    }
    return TRUE;
}
