/*
 * global-lifters — bass.dll proxy
 * Spawns Lifters at player position
 *
 * Auto-generated from CEA script.
 * Export g_spawn_flag (set to 1 via CE hotkey to spawn at player position)
 */

#include "bass_proxy.h"

/* Game function typedefs */
typedef void* (__cdecl *operator_new_t)(SIZE_T);
static operator_new_t pfn_operator_new = (operator_new_t)0x004BA570;

typedef void* (__thiscall *meshworld_ctor_t)(void* mem, void* graphics, const char* filename);
static meshworld_ctor_t pfn_meshworld_ctor = (meshworld_ctor_t)0x00461510;

/* Constructor: __thiscall, ECX=mem, stack params, RET 0x14 */
typedef void* (__thiscall *obj_ctor_t)(void* mem, void* board, float x, float y, float z, void* mesh);
static obj_ctor_t pfn_obj_ctor = (obj_ctor_t)0x457AD0;

typedef void (__thiscall *athena_append_t)(void* list, int item);
static athena_append_t pfn_append = (athena_append_t)0x00453810;

__declspec(dllexport) volatile DWORD g_spawn_flag = 0;

static void* g_mesh = NULL;

static void* load_mesh(void) {
    if (g_mesh) return g_mesh;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || IsBadReadPtr((void*)(app + 0x174), 4)) return NULL;
    void* gfx = *(void**)(app + 0x174);
    if (!gfx) return NULL;
    void* mem = pfn_operator_new(0x10D0);
    if (!mem) return NULL;
    g_mesh = pfn_meshworld_ctor(mem, gfx, "None");
    return g_mesh;
}

static void spawn_at_player(void) {
    DWORD board = get_board();
    if (!board) return;
    
    DWORD ball = get_player_ball();
    if (!ball) return;
    
    float x, y, z;
    get_ball_pos(ball, &x, &y, &z);
    
    void* mesh = load_mesh();
    if (!mesh) return;
    
    void* mem = pfn_operator_new(0x10D0);
    if (!mem) return;
    
    void* obj = pfn_obj_ctor(mem, (void*)board, x, y, z, mesh);
    if (!obj) return;
    
    pfn_append((void*)(board + 0x2578), (int)obj);
}

/* Ball_Update hook at 0x405E22 */
static unsigned char g_orig_bytes[6];
static void* g_patch_addr = (void*)0x00405E22;
static unsigned char* g_cave = NULL;
static int g_hooked = 0;

static void __cdecl on_ball_update(void) {
    if (g_spawn_flag) {
        g_spawn_flag = 0;
        spawn_at_player();
    }
}

static void install_hook(void) {
    if (g_hooked) return;
    memcpy(g_orig_bytes, g_patch_addr, 6);
    
    g_cave = (unsigned char*)alloc_executable(256);
    if (!g_cave) return;
    
    int i = 0;
    g_cave[i++] = 0x9C;  /* PUSHFD */
    g_cave[i++] = 0x60;  /* PUSHAD */
    g_cave[i++] = 0xE8;  /* CALL on_ball_update */
    *(int*)(g_cave + i) = (int)&on_ball_update - ((int)g_cave + i + 4);
    i += 4;
    g_cave[i++] = 0x61;  /* POPAD */
    g_cave[i++] = 0x9D;  /* POPFD */
    memcpy(g_cave + i, g_orig_bytes, 6);
    i += 6;
    g_cave[i++] = 0xE9;  /* JMP back */
    *(int*)(g_cave + i) = ((int)g_patch_addr + 6) - ((int)g_cave + i + 4);
    i += 4;
    
    DWORD old;
    VirtualProtect(g_patch_addr, 6, PAGE_EXECUTE_READWRITE, &old);
    *(unsigned char*)g_patch_addr = 0xE9;
    *(int*)((unsigned char*)g_patch_addr + 1) = (int)g_cave - ((int)g_patch_addr + 5);
    *(unsigned char*)((char*)g_patch_addr + 5) = 0x90;
    VirtualProtect(g_patch_addr, 6, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_patch_addr, 6);
    
    g_hooked = 1;
}

static DWORD WINAPI mod_thread(LPVOID param) {
    Sleep(3000);
    load_real_bass();
    install_hook();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
    }
    return TRUE;
}
