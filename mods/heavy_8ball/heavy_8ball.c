/*
 * heavy_8ball.c — BASS.dll proxy mod (v2)
 *
 * Play as the 8-Ball! Changes your player ball to use the 8-ball mesh.
 *
 * v2 fixes: Uses App→profile→board chain for reliable ball detection
 * on Wine/Winlator. Strips mass changes — just the mesh swap.
 *
 * Android-safe: No IAT hooks, no code caves, no VirtualProtect.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll heavy_8ball.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
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
        #define LOAD(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)
        LOAD(BASS_ChannelSetAttributes);
        LOAD(BASS_MusicPlayEx);
        LOAD(BASS_SetConfig);
        LOAD(BASS_Init);
        LOAD(BASS_Free);
        LOAD(BASS_Start);
        LOAD(BASS_Stop);
        LOAD(BASS_ErrorGetCode);
        LOAD(BASS_MusicLoad);
        LOAD(BASS_ChannelStop);
        #undef LOAD
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * 8-Ball Mesh Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_PTR_ADDR   0x005341E0

/* Ball struct offsets */
#define BALL_PLAYER_INDEX  0x018   /* 0 = player 1 */
#define BALL_MESH_INDEX    0x754   /* 0=Sphere, 9=8Ball */

/* App struct offsets (verified via decompilation) */
#define APP_PROFILE         0x220   /* App+0x220 = PlayerProfile* */
#define PROFILE_BOARD       0x00C   /* profile+0xC = Board* */

/* Board struct offsets */
#define BOARD_BALL_LIST     0x29D4  /* AthenaList of balls */
#define BOARD_ALL_BALLS     0x2DEC  /* AthenaList of all balls (backup) */

/* AthenaList struct offsets */
#define ATHENA_VTABLE       0x000
#define ATHENA_COUNT        0x004
#define ATHENA_ITEMS_INLINE 0x008   /* items when count==1 */
#define ATHENA_ITEMS_PTR    0x008   /* pointer to items array when count>1 */

/* Mesh indices */
#define MESH_8BALL          9

/* Scene vtable for verification */
#define SCENE_VTABLE        0x004D0260
#define BALL_VTABLE_ADDR    0x004CF3A0

/* Background thread: set player 1's ball mesh to 8Ball */
static DWORD WINAPI eight_ball_thread(LPVOID param)
{
    Sleep(5000);  /* Wait for game to fully load */

    for (;;) {
        Sleep(33);  /* ~30fps polling */

        /* Step 1: Get App */
        DWORD *pAppPtr = (DWORD*)APP_PTR_ADDR;
        if (IsBadReadPtr(pAppPtr, 4)) continue;
        DWORD app = *pAppPtr;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x400)) continue;

        /* Step 2: App→profile→board chain (verified method) */
        DWORD profile = *(DWORD*)((BYTE*)app + APP_PROFILE);
        if (!profile || profile < 0x10000) continue;
        if (IsBadReadPtr((void*)profile, 0x20)) continue;

        DWORD board = *(DWORD*)((BYTE*)profile + PROFILE_BOARD);
        if (!board || board < 0x10000) continue;
        if (IsBadReadPtr((void*)board, 0x3000)) continue;

        /* Step 3: Scan ball list for player 1 */
        DWORD list_addr = board + BOARD_BALL_LIST;
        if (IsBadReadPtr((void*)list_addr, 0x10)) continue;

        int count = *(int*)(list_addr + ATHENA_COUNT);
        if (count <= 0 || count > 100) {
            /* Try the all-balls list as fallback */
            list_addr = board + BOARD_ALL_BALLS;
            if (IsBadReadPtr((void*)list_addr, 0x10)) continue;
            count = *(int*)(list_addr + ATHENA_COUNT);
            if (count <= 0 || count > 100) continue;
        }

        /* Get items array */
        DWORD *items = *(DWORD**)(list_addr + ATHENA_ITEMS_PTR);
        if (!items || IsBadReadPtr((void*)items, count * 4)) continue;

        /* Find player 1's ball (player_index == 0) */
        for (int i = 0; i < count; i++) {
            if (IsBadReadPtr((void*)&items[i], 4)) break;
            DWORD ball = items[i];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0xD00)) continue;

            /* Verify it's a Ball (vtable check) */
            DWORD vt = *(DWORD*)ball;
            if (vt != BALL_VTABLE_ADDR) continue;

            int pidx = *(int*)((BYTE*)ball + BALL_PLAYER_INDEX);
            if (pidx == 0) {
                /* Found player 1 — set mesh to 8Ball */
                if (!IsBadWritePtr((void*)(ball + BALL_MESH_INDEX), 4)) {
                    *(int*)((BYTE*)ball + BALL_MESH_INDEX) = MESH_8BALL;
                }
                break;
            }
        }
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        {
            HANDLE hThread = CreateThread(NULL, 0, eight_ball_thread, NULL, 0, NULL);
            if (hThread) CloseHandle(hThread);
        }
        break;
    }
    return TRUE;
}
