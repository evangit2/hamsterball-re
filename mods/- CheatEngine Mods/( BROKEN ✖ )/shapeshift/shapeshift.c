/*
 * shapeshift — bass.dll proxy
 * Swaps player ball mesh to 8Ball every frame.
 * Auto-generated from Shapeshift.CEA v12.
 */

#include "bass_proxy.h"

/* Ball_Update hook at 0x405E22 (ESI=ball, 6 bytes: 8B 86 5C 0C 00 00) */
static unsigned char g_orig_bytes[6];
static void* g_patch_addr = (void*)0x00405E22;
static unsigned char* g_cave = NULL;
static int g_hooked = 0;

static void __cdecl on_ball_update(void) {
    /* ESI = ball pointer, but we can't access it from C directly.
     * Instead, use the board mesh array approach: swap board+0x244[0] ↔ board+0x268 */
    DWORD board = get_board();
    if (!board || IsBadReadPtr((void*)(board + 0x244), 8)) return;
    if (IsBadReadPtr((void*)(board + 0x268), 4)) return;
    
    DWORD* mesh_array = (DWORD*)(board + 0x244);
    DWORD sphere_mesh = mesh_array[0];
    DWORD eightball_mesh = *(DWORD*)(board + 0x268);
    
    if (sphere_mesh && eightball_mesh) {
        mesh_array[0] = eightball_mesh;
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
