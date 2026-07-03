/*
 * global-blockdawg — bass.dll proxy
 * Global Blockdawg v14 — PopCylinder_ctor + vtable swap
 *
 * Export g_spawn_flag (set to 1 via CE hotkey to spawn at player position)
 */

#include "bass_proxy.h"

/* Game function typedefs */
typedef void* (__cdecl *operator_new_t)(SIZE_T);
static operator_new_t pfn_new = (operator_new_t)0x004BA570;

typedef void (__thiscall *athena_append_t)(void* list, int item);
static athena_append_t pfn_append = (athena_append_t)0x00453810;

/* Constructor: __thiscall, ECX=mem, stack=[board,x,y,z,mesh], RET 0x14 */
typedef void* (__thiscall *obj_ctor_t)(void* mem, void* board, float x, float y, float z, void* mesh);
static obj_ctor_t pfn_ctor = (obj_ctor_t)0x436EE0;

__declspec(dllexport) volatile DWORD g_spawn_flag = 0;

static void spawn_at_player(void) {
    DWORD board = get_board();
    if (!board) return;
    DWORD ball = get_player_ball();
    if (!ball) return;
    
    float x, y, z;
    get_ball_pos(ball, &x, &y, &z);
    
    void* mesh = NULL;
    /* Use board's pre-loaded mesh if available */
    if (!IsBadReadPtr((void*)(board + 0x4370), 4)) {
        mesh = *(void**)(board + 0x4370);
    }
    if (!mesh && !IsBadReadPtr((void*)(board + 0x4374), 4)) {
        mesh = *(void**)(board + 0x4374);
    }
    
    void* mem = pfn_new(0x10E8);
    if (!mem) return;
    
    void* obj = pfn_ctor(mem, (void*)board, x, y, z, mesh);
    if (!obj) return;
    
    /* Swap vtable to Blockdawg */
    if (!IsBadWritePtr((void*)obj, 4)) {
        *(DWORD*)obj = 0x4D5638;
    }
    /* Prevent NULL sound crashes */
    if (!IsBadWritePtr((void*)((DWORD)obj + 0x1148), 4)) {
        *(int*)((DWORD)obj + 0x1148) = 0x7FFFFFFF;
    }
    if (!IsBadWritePtr((void*)((DWORD)obj + 0x1151), 1)) {
        *(BYTE*)((DWORD)obj + 0x1151) = 1;
    }
    pfn_append((void*)(board + 0x2578), (int)obj);
}

/* Ball_Update hook at 0x405E22 */
static unsigned char g_orig[6];
static void* g_hook = (void*)0x00405E22;
static unsigned char* g_cave = NULL;
static int g_installed = 0;

static void __cdecl on_frame(void) {
    if (g_spawn_flag) {
        g_spawn_flag = 0;
        spawn_at_player();
    }

}

static void install_hook(void) {
    if (g_installed) return;
    memcpy(g_orig, g_hook, 6);
    g_cave = alloc_executable(256);
    if (!g_cave) return;
    int i = 0;
    g_cave[i++] = 0x9C;  /* PUSHFD */
    g_cave[i++] = 0x60;  /* PUSHAD */
    g_cave[i++] = 0xE8;  /* CALL on_frame */
    *(int*)(g_cave + i) = (int)&on_frame - ((int)g_cave + i + 4);
    i += 4;
    g_cave[i++] = 0x61;  /* POPAD */
    g_cave[i++] = 0x9D;  /* POPFD */
    memcpy(g_cave + i, g_orig, 6);
    i += 6;
    g_cave[i++] = 0xE9;
    *(int*)(g_cave + i) = ((int)g_hook + 6) - ((int)g_cave + i + 4);
    DWORD old;
    VirtualProtect(g_hook, 6, PAGE_EXECUTE_READWRITE, &old);
    *(unsigned char*)g_hook = 0xE9;
    *(int*)((unsigned char*)g_hook + 1) = (int)g_cave - ((int)g_hook + 5);
    *(unsigned char*)((char*)g_hook + 5) = 0x90;
    VirtualProtect(g_hook, 6, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_hook, 6);
    g_installed = 1;
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
