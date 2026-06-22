
// ============================================================
// SpeedCylinder Spawner — bass.dll proxy v1
// Spawns SpeedCylinder objects globally at Player 1's position.
//
// HOW IT WORKS:
//   1. On first spawn request, loads the SpeedCylinder MeshWorld:
//      operator_new(0x10d0) → MeshWorld_ctor(mem, graphics, "levels\\levelup-speedcylinder")
//   2. On spawn trigger (g_spawn_flag=1), creates a Pendulum (SpeedCylinder):
//      operator_new(0x150c) → Pendulum_ctor(mem, scene, x, y, z, 0, meshWorld*)
//      → AthenaList_Append(scene+0x2578, pendulum)
//   3. Automatically resets g_spawn_flag to 0 after spawning.
//
// In Cheat Engine:
//   - Add address: "spawnSpeed" → points to g_spawn_flag
//   - Set hotkey: when a key is pressed, set value to 1
//   - The DLL checks this flag each frame via Graphics_PresentOrEnd hook
//
// BUILD:
//   i686-w64-mingw32-gcc -shared -o bass.dll speedcyl_spawn.c \
//       bass_exports.def -lwinmm -Wl,--enable-stdcall-fixup \
//       -O2 -static -static-libgcc -Wl,--add-stdcall-alias
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward all game imports to bass_real.dll
 * Requires: rename original bass.dll → bass_real.dll in game folder
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

/* Extra stubs — forward if available, otherwise return safe defaults */
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
        real_BASS_ChannelStop           = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Constants & Struct Offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE        0x400000

/* Global App pointer (BSS, filled at runtime) */
#define APP_PTR_ADDR      0x005341E0

/* Function addresses (VA) */
#define OPERATOR_NEW      0x004BA57B  /* __cdecl: size_t → void* */
#define MESHWORLD_CTOR    0x00461510  /* __thiscall: ECX=mem, stack=[graphics, char*] → RET 8 */
#define PENDULUM_CTOR     0x00436A20  /* __thiscall: ECX=mem, stack=[scene,x,y,z,int,mesh*] → RET 0x18 */
#define ATHENA_LIST_APPEND 0x00453780 /* __thiscall: ECX=list, stack=[item] → RET 4 */
#define GFX_PRESENT_OR_END 0x00455A90 /* __thiscall: hook point */
#define GFX_SET_POSITION   0x00457B50 /* __thiscall: ECX=sceneobj, stack=[x,y,z] → RET 0xC */

/* App struct offsets */
#define APP_GRAPHICS      0x174       /* Graphics* */
#define APP_SCENE         0x178       /* Scene/Board* */

/* Scene struct offsets */
#define SCENE_OBJECT_LIST 0x2578      /* AthenaList — all game objects */
#define SCENE_ALL_BALLS   0x2DEC      /* AthenaList — all balls */

/* AthenaList struct offsets */
#define ATHENA_LIST_COUNT  0x004      /* int: element count */
#define ATHENA_LIST_ARRAY  0x40C      /* int*: heap-allocated element array */

/* Ball struct offsets */
#define BALL_SCENE        0x014       /* Scene/Board* */
#define BALL_PLAYER_IDX   0x018       /* int: -1=NPC, 0-3=Player 1-4 */
#define BALL_POS_X        0x164       /* float */
#define BALL_POS_Y        0x168       /* float */
#define BALL_POS_Z        0x16C       /* float */

/* Allocation sizes */
#define MESHWORLD_SIZE    0x10D0      /* MeshWorld allocation */
#define PENDULUM_SIZE     0x150C      /* Pendulum (SpeedCylinder) allocation */

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Function Wrappers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* operator_new(size_t) — __cdecl, returns void* in EAX */
typedef void* (__cdecl *operator_new_t)(size_t);
static operator_new_t pfn_operator_new = NULL;

/* MeshWorld_ctor(void* mem, void* graphics, char* filename) — __thiscall
 * ECX=mem, stack=[graphics, filename], RET 8 */
typedef void* (__thiscall *meshworld_ctor_t)(void* mem, void* graphics, const char* filename);
static meshworld_ctor_t pfn_meshworld_ctor = NULL;

/* Pendulum_ctor(void* mem, void* scene, float x, float y, float z, int param5, void* mesh)
 * __thiscall: ECX=mem, stack=[scene, x, y, z, param5, mesh], RET 0x18 */
typedef void* (__thiscall *pendulum_ctor_t)(void* mem, void* scene,
    float x, float y, float z, int param5, void* mesh);
static pendulum_ctor_t pfn_pendulum_ctor = NULL;

/* AthenaList_Append(void* list, int item) — __thiscall: ECX=list, stack=[item], RET 4 */
typedef void (__thiscall *athena_list_append_t)(void* list, int item);
static athena_list_append_t pfn_athena_list_append = NULL;

static void resolve_functions(void)
{
    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return;
    BYTE *base = (BYTE*)hExe;
    pfn_operator_new = (operator_new_t)(base + (OPERATOR_NEW - IMAGE_BASE));
    pfn_meshworld_ctor = (meshworld_ctor_t)(base + (MESHWORLD_CTOR - IMAGE_BASE));
    pfn_pendulum_ctor = (pendulum_ctor_t)(base + (PENDULUM_CTOR - IMAGE_BASE));
    pfn_athena_list_append = (athena_list_append_t)(base + (ATHENA_LIST_APPEND - IMAGE_BASE));
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Spawn State
 * ═══════════════════════════════════════════════════════════════════════════ */

/* This is the flag CE sets via hotkey. When set to 1, the per-frame
 * hook spawns a SpeedCylinder at Player 1's position, then resets to 0.
 * EXPORTED so Cheat Engine can access it as bass.dll!g_spawn_flag */
__declspec(dllexport) volatile DWORD g_spawn_flag = 0;

/* Cached MeshWorld pointer for SpeedCylinder mesh */
static void* g_speedcyl_mesh = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Helper: Find Player 1 ball position
 * ═══════════════════════════════════════════════════════════════════════════ */

static int find_player1_pos(void* scene, float* out_x, float* out_y, float* out_z)
{
    if (!scene) return 0;
    BYTE *s = (BYTE*)scene;
    int count = *(int*)(s + SCENE_ALL_BALLS + ATHENA_LIST_COUNT);
    int *array = *(int**)(s + SCENE_ALL_BALLS + ATHENA_LIST_ARRAY);
    if (!array || count <= 0) return 0;

    for (int i = 0; i < count; i++) {
        void *ball = (void*)array[i];
        if (!ball) continue;
        if (IsBadReadPtr(ball, 0x200)) continue;
        int idx = *(int*)((BYTE*)ball + BALL_PLAYER_IDX);
        if (idx == 0) {
            *out_x = *(float*)((BYTE*)ball + BALL_POS_X);
            *out_y = *(float*)((BYTE*)ball + BALL_POS_Y);
            *out_z = *(float*)((BYTE*)ball + BALL_POS_Z);
            return 1;
        }
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Core: Load SpeedCylinder mesh (once)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* load_speedcyl_mesh(void* graphics)
{
    if (g_speedcyl_mesh) return g_speedcyl_mesh;
    if (!graphics) return NULL;

    void *mem = pfn_operator_new(MESHWORLD_SIZE);
    if (!mem) return NULL;

    /* MeshWorld_ctor(mem, graphics, "levels\\levelup-speedcylinder") */
    void *result = pfn_meshworld_ctor(mem, graphics, "levels\\levelup-speedcylinder");
    if (!result) {
        /* operator_new returned NULL or ctor failed — can't free, just abort */
        return NULL;
    }
    g_speedcyl_mesh = result;
    return g_speedcyl_mesh;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Core: Spawn one SpeedCylinder at position
 * ═══════════════════════════════════════════════════════════════════════════ */

static void spawn_speedcylinder(void* scene, void* mesh, float x, float y, float z)
{
    if (!scene || !mesh) return;

    /* 1. Allocate Pendulum memory */
    void *mem = pfn_operator_new(PENDULUM_SIZE);
    if (!mem) return;

    /* 2. Construct Pendulum (SpeedCylinder):
     * Pendulum_ctor(mem, scene, x, y, z, 0, mesh)
     * __thiscall: ECX=mem, stack=[scene, x, y, z, 0, mesh] */
    void *pendulum = pfn_pendulum_ctor(mem, scene, x, y, z, 0, mesh);
    if (!pendulum) return;

    /* 3. Add to Scene's object list (Scene+0x2578) */
    BYTE *s = (BYTE*)scene;
    pfn_athena_list_append(s + SCENE_OBJECT_LIST, (int)pendulum);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-Frame Hook: Graphics_PresentOrEnd
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Original 5 bytes at Graphics_PresentOrEnd entry (0x455A90)
 * We need to read these from the live process at patch time.
 * The function starts with: PUSH EBX; PUSH ESI; ...
 * Standard SEH prolog. We'll use a 5-byte JMP to our cave. */

static unsigned char g_orig_bytes[5];
static void* g_patch_addr = NULL;
static int g_patched = 0;

/* Our per-frame callback */
static void __cdecl on_frame(void)
{
    if (g_spawn_flag) {
        g_spawn_flag = 0;  /* Reset immediately */

        /* Get App pointer */
        DWORD app = *(DWORD*)APP_PTR_ADDR;
        if (!app) return;

        /* Get graphics and scene from App */
        void *graphics = *(void**)((BYTE*)app + APP_GRAPHICS);
        void *scene = *(void**)((BYTE*)app + APP_SCENE);
        if (!scene) return;

        /* Load SpeedCylinder mesh (cached) */
        void *mesh = load_speedcyl_mesh(graphics);
        if (!mesh) return;

        /* Find Player 1 position */
        float x, y, z;
        if (find_player1_pos(scene, &x, &y, &z)) {
            spawn_speedcylinder(scene, mesh, x, y, z);
        }
    }
}

/* Code cave: saves registers, calls on_frame, restores, runs original bytes, jumps back */
static unsigned char* g_cave = NULL;

static void BuildFrameHook(void)
{
    /* Save original bytes */
    g_patch_addr = (void*)(IMAGE_BASE + (GFX_PRESENT_OR_END - IMAGE_BASE));
    memcpy(g_orig_bytes, g_patch_addr, 5);

    /* Allocate code cave */
    g_cave = (unsigned char*)VirtualAlloc(NULL, 256, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memset(g_cave, 0x90, 256);

    int i = 0;

    /* PUSHFD; PUSHAD — save all registers + flags */
    g_cave[i++] = 0x9C;  /* PUSHFD */
    g_cave[i++] = 0x60;  /* PUSHAD */

    /* CALL on_frame — relative call */
    g_cave[i++] = 0xE8;  /* CALL rel32 */
    int call_offset = (int)&on_frame - ((int)g_cave + i + 4);
    *(int*)(g_cave + i) = call_offset;
    i += 4;

    /* POPAD; POPFD — restore registers + flags */
    g_cave[i++] = 0x61;  /* POPAD */
    g_cave[i++] = 0x9D;  /* POPFD */

    /* Execute original 5 bytes */
    memcpy(g_cave + i, g_orig_bytes, 5);
    i += 5;

    /* JMP back to original_addr + 5 */
    g_cave[i++] = 0xE9;  /* JMP rel32 */
    DWORD ret_addr = (DWORD)g_patch_addr + 5;
    *(int*)(g_cave + i) = (int)ret_addr - ((int)g_cave + i + 4);
    i += 4;

    /* Install jump at Graphics_PresentOrEnd entry */
    DWORD old;
    VirtualProtect(g_patch_addr, 5, PAGE_EXECUTE_READWRITE, &old);
    g_orig_bytes[0] = ((unsigned char*)g_patch_addr)[0];
    g_orig_bytes[1] = ((unsigned char*)g_patch_addr)[1];
    g_orig_bytes[2] = ((unsigned char*)g_patch_addr)[2];
    g_orig_bytes[3] = ((unsigned char*)g_patch_addr)[3];
    g_orig_bytes[4] = ((unsigned char*)g_patch_addr)[4];

    /* Write JMP to cave */
    ((unsigned char*)g_patch_addr)[0] = 0xE9;
    *(int*)((unsigned char*)g_patch_addr + 1) = (int)g_cave - ((int)g_patch_addr + 5);
    VirtualProtect(g_patch_addr, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_patch_addr, 5);

    g_patched = 1;
}

static void RemoveFrameHook(void)
{
    if (!g_patched) return;
    DWORD old;
    VirtualProtect(g_patch_addr, 5, PAGE_EXECUTE_READWRITE, &old);
    memcpy(g_patch_addr, g_orig_bytes, 5);
    VirtualProtect(g_patch_addr, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_patch_addr, 5);
    if (g_cave) { VirtualFree(g_cave, 0, MEM_RELEASE); g_cave = NULL; }
    g_patched = 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI PatchThread(LPVOID lpParam)
{
    Sleep(2000);  /* Wait for game to fully load */

    load_real_bass();
    resolve_functions();

    /* Verify function pointers resolved */
    if (!pfn_operator_new || !pfn_meshworld_ctor || !pfn_pendulum_ctor || !pfn_athena_list_append)
        return 0;

    BuildFrameHook();

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);
            CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
            break;
        case DLL_PROCESS_DETACH:
            RemoveFrameHook();
            break;
    }
    return TRUE;
}
