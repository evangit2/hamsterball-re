/*
 * Hamsterball FPS Unlock DLL
 *
 * Removes the game's built-in frame rate caps:
 *   1. App+0x16C (target_fps=100)  → set to 1000 (unlimited update rate)
 *   2. App+0x170 (render_fps=75)    → set to 1000 (unlimited render rate)
 *   3. Render-skip conditional at 0x46BF55 → NOP'd (always render)
 *
 * The game normally caps updates at 100 Hz and rendering at 75 Hz.
 * This DLL patches the values at runtime after the game initializes,
 * allowing uncapped frame rates up to 1000 FPS (limited only by
 * hardware + vsync driver setting).
 *
 * Build: i686-w64-mingw32-gcc -shared -o fps_unlock.dll fps_unlock.c \
 *          -Wl,--enable-stdcall-fixup
 *
 * Usage: Inject into Hamsterball.exe using any DLL injector.
 *        Or rename to dinput8.dll for auto-load (proxy stub).
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define GAME_BASE 0x0040000000
#define GAME_BASE_REAL 0x00400000

/* App struct offsets (from App_Ctor at 0x46DC40) */
#define APP_TARGET_FPS_OFFSET  0x16C   /* int32, default 100 */
#define APP_RENDER_FPS_OFFSET  0x170   /* int32, default 75  */

/* Address of the render-skip JBE in App_Run (0x46BF55) */
#define RENDER_SKIP_JBE_ADDR   0x0046BF55
#define RENDER_SKIP_JBE_BYTES  2       /* 76 5D = JBE +0x5D */

/* App_Ctor address — we hook AFTER App_Run starts to ensure App is fully initialized */
#define APP_RUN_ADDR 0x0046BD80

/* New FPS values */
#define NEW_TARGET_FPS  1000
#define NEW_RENDER_FPS  1000

/* ── Globals ─────────────────────────────────────────────────────────── */

static HMODULE g_hSelf = NULL;
static int g_patched = 0;

/* Original bytes for restoring (not really needed but good practice) */
static unsigned char g_orig_jbe[2];
static int g_have_orig = 0;

/* ── FPS Unlock Logic ────────────────────────────────────────────────── */

static DWORD WINAPI unlock_thread(LPVOID lpParam) {
    (void)lpParam;

    /* Wait for the game to finish initializing */
    /* App_Run is called from WinMain after App_Initialize_Full completes */
    Sleep(3000);

    /* Get actual module base (handles ASLR, though Hamsterball likely doesn't use it) */
    HMODULE hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)(uintptr_t)hExe;
    DWORD offset = base - GAME_BASE_REAL;

    /* ── Patch 1: App+0x16C (target FPS) ──────────────────────────── */
    /* App_Ctor sets param_1[0x5b]=100 (App+0x16C) and param_1[0x5c]=0x4b (App+0x170).
     * param_1 is int*, so param_1[0x5b] = byte offset 0x5b*4 = 0x16C. Correct.
     * g_App pointer is stored at DAT_005341E0 (set in App_Ctor).
     */

    DWORD appAddr = 0;
    DWORD *pAppPtr = (DWORD *)(0x005341E0 + offset);
    if (!IsBadReadPtr(pAppPtr, 4)) {
        appAddr = *pAppPtr;
    }

    DWORD oldProtect;
    if (appAddr != 0 && !IsBadWritePtr((void *)appAddr, 0x200)) {
        DWORD *pTargetFPS = (DWORD *)(appAddr + APP_TARGET_FPS_OFFSET);
        DWORD *pRenderFPS = (DWORD *)(appAddr + APP_RENDER_FPS_OFFSET);

        if (VirtualProtect(pTargetFPS, 4, PAGE_READWRITE, &oldProtect)) {
            *pTargetFPS = NEW_TARGET_FPS;
            VirtualProtect(pTargetFPS, 4, oldProtect, &oldProtect);
        }
        if (VirtualProtect(pRenderFPS, 4, PAGE_READWRITE, &oldProtect)) {
            *pRenderFPS = NEW_RENDER_FPS;
            VirtualProtect(pRenderFPS, 4, oldProtect, &oldProtect);
        }
    }

    /* ── Patch 2: NOP the render-skip JBE at 0x46BF55 ──────────────── */
    {
        unsigned char *pJBE = (unsigned char *)(RENDER_SKIP_JBE_ADDR + offset);
        if (VirtualProtect(pJBE, RENDER_SKIP_JBE_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            if (!g_have_orig) {
                memcpy(g_orig_jbe, pJBE, RENDER_SKIP_JBE_BYTES);
                g_have_orig = 1;
            }
            pJBE[0] = 0x90;
            pJBE[1] = 0x90;
            VirtualProtect(pJBE, RENDER_SKIP_JBE_BYTES, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), pJBE, RENDER_SKIP_JBE_BYTES);
        }
    }

    g_patched = 1;

    /* Write a log file so the user knows it worked */
    {
        FILE *f = fopen("fps_unlock.log", "w");
        if (f) {
            fprintf(f, "Hamsterball FPS Unlock v1.0\n");
            fprintf(f, "==========================\n");
            fprintf(f, "Module base: 0x%08lX (offset 0x%08lX)\n", (unsigned long)base, (unsigned long)offset);
            fprintf(f, "App pointer: 0x%08lX\n", (unsigned long)appAddr);
            fprintf(f, "Target FPS: 100 -> %d (App+0x16C)\n", NEW_TARGET_FPS);
            fprintf(f, "Render FPS: 75  -> %d (App+0x170)\n", NEW_RENDER_FPS);
            fprintf(f, "Render-skip JBE at 0x%08lX: NOP'd\n", (unsigned long)(RENDER_SKIP_JBE_ADDR + offset));
            fprintf(f, "Status: ACTIVE - FPS cap removed\n");
            fclose(f);
        }
    }

    return 0;
}

/* ── DLL Entry Point ─────────────────────────────────────────────────── */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        g_hSelf = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, unlock_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        /* Restore original bytes on unload */
        if (g_patched && g_have_orig) {
            HMODULE hExe = GetModuleHandleA(NULL);
            DWORD base = (DWORD)(uintptr_t)hExe;
            DWORD offset = base - GAME_BASE_REAL;
            unsigned char *pJBE = (unsigned char *)(RENDER_SKIP_JBE_ADDR + offset);
            DWORD oldProtect;
            if (VirtualProtect(pJBE, RENDER_SKIP_JBE_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                pJBE[0] = g_orig_jbe[0];
                pJBE[1] = g_orig_jbe[1];
                VirtualProtect(pJBE, RENDER_SKIP_JBE_BYTES, oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), pJBE, RENDER_SKIP_JBE_BYTES);
            }
        }
        break;
    }
    return TRUE;
}

/* Exported for manual invocation */
__declspec(dllexport) void __cdecl InitUnlock(void) {
    unlock_thread(NULL);
}
