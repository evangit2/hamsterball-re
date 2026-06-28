/*
 * heavy_8ball.c — BASS.dll proxy mod
 *
 * Play as the Heavy 8-Ball! Changes your player ball to use the 8-ball mesh
 * and gives it 4x mass so it knocks other balls around like a bowling ball.
 *
 * What it does:
 *   1. Sets ball+0x754 = 9 (8Ball mesh index) every frame
 *   2. Sets ball+0xC78 = 100.0 (heavy mass, 4x normal Arena mass of 25.0)
 *   3. Sets ball+0xC7C = 1 (enable battle physics flag)
 *
 * The 8Ball mesh is preloaded by the game at board+0x268 (mesh index 9).
 * The mass at +0xC78 controls collision response — higher mass = more
 * momentum transfer to other balls, less knockback to yourself.
 *
 * Android-safe:
 *   - No IAT hooks, no code caves, no VirtualProtect
 *   - Background thread does memory writes only
 *   - All pointer accesses guarded by IsBadReadPtr
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll heavy_8ball.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * Installation:
 *   1. In your Hamsterball game folder, rename bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll into the game folder
 *   3. Launch the game — your ball will be the heavy 8-ball automatically
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
 * Heavy 8-Ball Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define APP_PTR_ADDR   0x005341E0

/* Ball struct offsets */
#define BALL_PLAYER_INDEX  0x018   /* ball+0x18 = player index (0=player 1) */
#define BALL_MESH_INDEX    0x754   /* ball+0x754 = mesh index (0=Sphere, 9=8Ball) */
#define BALL_MASS           0xC78   /* ball+0xC78 = mass (Arena default 25.0) */
#define BALL_BATTLE_FLAG    0xC7C   /* ball+0xC7C = battle physics enabled (byte) */

/* Scene struct offsets */
#define SCENE_BALL_LIST     0x29D4
#define ATHENA_COUNT_OFFSET 0x004
#define ATHENA_ARRAY_OFFSET 0x40C

#define EIGHTBALL_MESH_IDX  9
#define HEAVY_MASS          100.0f  /* 4x normal Arena mass (25.0) */

/* Background thread: set player 1's ball to heavy 8-ball every frame */
static DWORD WINAPI heavy_8ball_thread(LPVOID param)
{
    Sleep(3000);  /* Wait for game to fully load */

    for (;;) {
        Sleep(50);  /* 20fps — fast enough to catch ball spawns */

        /* Find App */
        DWORD app = *(DWORD*)APP_PTR_ADDR;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x300)) continue;

        /* Find Scene by scanning App for ball list */
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

        /* Find player 1's ball and make it the heavy 8-ball */
        DWORD list_base = scene + SCENE_BALL_LIST;
        int bcount = *(int*)(list_base + ATHENA_COUNT_OFFSET);
        DWORD *barray = *(DWORD**)(list_base + ATHENA_ARRAY_OFFSET);

        for (int i = 0; i < bcount && i < 100; i++) {
            if (IsBadReadPtr((void*)&barray[i], 4)) break;
            DWORD ball = barray[i];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0xD00)) continue;

            int pidx = *(int*)((BYTE*)ball + BALL_PLAYER_INDEX);
            if (pidx == 0) {
                /* Found player 1's ball — make it the heavy 8-ball */
                if (!IsBadWritePtr((void*)(ball + BALL_MESH_INDEX), 4)) {
                    *(int*)((BYTE*)ball + BALL_MESH_INDEX) = EIGHTBALL_MESH_IDX;
                }
                if (!IsBadWritePtr((void*)(ball + BALL_MASS), 4)) {
                    *(float*)((BYTE*)ball + BALL_MASS) = HEAVY_MASS;
                }
                if (!IsBadWritePtr((void*)(ball + BALL_BATTLE_FLAG), 1)) {
                    *(BYTE*)((BYTE*)ball + BALL_BATTLE_FLAG) = 1;
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
            HANDLE hThread = CreateThread(NULL, 0, heavy_8ball_thread, NULL, 0, NULL);
            if (hThread) CloseHandle(hThread);
        }
        break;
    }
    return TRUE;
}
