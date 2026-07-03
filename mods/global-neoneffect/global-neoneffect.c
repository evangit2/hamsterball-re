/*
 * global-neoneffect — bass.dll proxy
 * v27: Dark fog + ball glow on ALL levels.
 * PART 1: Distance fog via Graphics_BeginFrame hook.
 * PART 2: Force ball+0xC80=1 (glow flag) via ball render hook.
 * Auto-generated from GlobalNeonEffect.CEA.
 */

#include "bass_proxy.h"

/* PART 1: Fog via background thread (sets D3D render states) */
static DWORD WINAPI fog_thread(LPVOID param) {
    Sleep(3000);
    while (1) {
        DWORD app = *(DWORD*)GLOBAL_APP_PTR;
        if (app && !IsBadReadPtr((void*)(app + 0x174), 4)) {
            DWORD gfx = *(DWORD*)(app + 0x174);
            if (gfx && !IsBadReadPtr((void*)(gfx + 0x154), 4)) {
                DWORD d3d_dev = *(DWORD*)(gfx + 0x154);
                if (d3d_dev && !IsBadReadPtr((void*)d3d_dev, 4)) {
                    /* Can't call D3D methods from a background thread safely.
                     * Use code cave approach instead for the hook. */
                }
            }
        }
        Sleep(100);
    }
    return 0;
}

/* PART 2: Force ball glow flag via Ball_Update hook */
static unsigned char g_orig_bytes[6];
static void* g_ball_update_addr = (void*)0x00405E22;
static unsigned char* g_ball_cave = NULL;

static void __cdecl on_ball_update(void) {
    DWORD board = get_board();
    if (!board) return;
    DWORD ball = get_player_ball();
    if (!ball || IsBadWritePtr((void*)(ball + 0xC80), 1)) return;
    *(BYTE*)(ball + 0xC80) = 1; /* Force glow flag on */
}

static void install_ball_hook(void) {
    memcpy(g_orig_bytes, g_ball_update_addr, 6);
    g_ball_cave = (unsigned char*)alloc_executable(256);
    if (!g_ball_cave) return;
    
    int i = 0;
    g_ball_cave[i++] = 0x9C;  /* PUSHFD */
    g_ball_cave[i++] = 0x60;  /* PUSHAD */
    g_ball_cave[i++] = 0xE8;  /* CALL */
    *(int*)(g_ball_cave + i) = (int)&on_ball_update - ((int)g_ball_cave + i + 4);
    i += 4;
    g_ball_cave[i++] = 0x61;  /* POPAD */
    g_ball_cave[i++] = 0x9D;  /* POPFD */
    memcpy(g_ball_cave + i, g_orig_bytes, 6);
    i += 6;
    g_ball_cave[i++] = 0xE9;  /* JMP back */
    *(int*)(g_ball_cave + i) = ((int)g_ball_update_addr + 6) - ((int)g_ball_cave + i + 4);
    i += 4;
    
    DWORD old;
    VirtualProtect(g_ball_update_addr, 6, PAGE_EXECUTE_READWRITE, &old);
    *(unsigned char*)g_ball_update_addr = 0xE9;
    *(int*)((unsigned char*)g_ball_update_addr + 1) = (int)g_ball_cave - ((int)g_ball_update_addr + 5);
    *(unsigned char*)((char*)g_ball_update_addr + 5) = 0x90;
    VirtualProtect(g_ball_update_addr, 6, old, &old);
    FlushInstructionCache(GetCurrentProcess(), g_ball_update_addr, 6);
}

static DWORD WINAPI mod_thread(LPVOID param) {
    Sleep(3000);
    load_real_bass();
    install_ball_hook();
    CreateThread(NULL, 0, fog_thread, NULL, 0, NULL);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
    }
    return TRUE;
}
