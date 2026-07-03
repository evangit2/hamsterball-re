/*
 * background-color-cycler — bass.dll proxy
 * Cycles the actual background color (Clear color) every ~3 seconds.
 * Hooks App_ResetFrame at 0x46C214 to inject cycled color into gfx+0x738.
 * Auto-generated from BackgroundColorCycler.CEA v3.
 */

#include "bass_proxy.h"

static const DWORD bg_colors[] = {
    0x000000FF, 0x000080FF, 0x008000FF, 0x808000FF,
    0xFF0000FF, 0xFF0080FF, 0xFF8000FF, 0xFFFFFF00,
    0x00FF00FF, 0x00FFFFFF, 0xFF00FFFF, 0xFF8080FF,
};
static const int num_colors = sizeof(bg_colors) / sizeof(bg_colors[0]);
static volatile DWORD g_color_index = 0;
static volatile DWORD g_frame_counter = 0;
static volatile DWORD g_cycle_timer = 180;

__declspec(dllexport) volatile DWORD* g_bg_colors_ptr = (DWORD*)bg_colors;
__declspec(dllexport) volatile int* g_num_colors_ptr = (int*)&num_colors;

/* Hook at 0x46C214: MOV EAX,[ECX+0x738] (6 bytes: 8B 81 38 07 00 00) */
static unsigned char g_orig_bytes[6];
static void* g_patch_addr = (void*)0x0046C214;
static unsigned char* g_cave = NULL;

static void __cdecl on_reset_frame(void) {
    g_frame_counter++;
    if (g_frame_counter >= g_cycle_timer) {
        g_frame_counter = 0;
        g_color_index = (g_color_index + 1) % num_colors;
    }
    /* Write our color to gfx+0x738 — ECX has gfx pointer in the original instruction */
    /* But we can't access ECX from C... we need to write via the code cave directly */
    /* Instead, poll the App struct and write gfx+0x738 from the background thread */
}

/* Background thread approach: write gfx+0x738 every frame */
static DWORD WINAPI bg_thread(LPVOID param) {
    Sleep(3000);
    while (1) {
        DWORD app = *(DWORD*)GLOBAL_APP_PTR;
        if (app && !IsBadReadPtr((void*)(app + 0x174), 4)) {
            DWORD gfx = *(DWORD*)(app + 0x174);
            if (gfx && !IsBadWritePtr((void*)(gfx + 0x738), 4)) {
                *(DWORD*)(gfx + 0x738) = bg_colors[g_color_index];
            }
        }
        g_frame_counter++;
        if (g_frame_counter >= g_cycle_timer) {
            g_frame_counter = 0;
            g_color_index = (g_color_index + 1) % num_colors;
        }
        Sleep(16); /* ~60fps */
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, bg_thread, NULL, 0, NULL);
    }
    return TRUE;
}
