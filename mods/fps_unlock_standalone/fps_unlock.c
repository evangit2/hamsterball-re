/*
 * Hamsterball FPS Unlock DLL v2 — Fixed Rotation Speed
 *
 * Removes the game's built-in RENDER frame rate cap while preserving
 * the original PHYSICS update rate. This decouples rendering from
 * physics, giving smooth high-FPS visuals without accelerating
 * ball rotation or other physics-driven animations.
 *
 * Root cause of v1 rotation bug:
 *   v1 set BOTH target_fps (physics) and render_fps to 1000.
 *   Ball_Update advances rotation by a FIXED increment per tick
 *   (not scaled by dt), so running physics at 1000Hz made the ball
 *   spin 10x faster.
 *
 * v2 fix: Only unlock render_fps. Keep target_fps at 100 (original).
 *   - App+0x16C (target_fps=100)  → LEFT UNCHANGED (physics stays 100Hz)
 *   - App+0x170 (render_fps=75)   → set to 1000 (unlimited render rate)
 *   - Render-skip JBE at 0x46BF55 → NOP'd (always render when possible)
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll fps_unlock.c \
 *          -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *          -Wl,--add-stdcall-alias
 *
 * Usage: Rename to bass.dll and place in Hamsterball game directory.
 *        (Original bass.dll must be renamed to bass_real.dll first.)
 *        Or inject into Hamsterball.exe using any DLL injector.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define GAME_BASE 0x0040000000
#define GAME_BASE_REAL 0x00400000

/* App struct offsets (from App_Ctor at 0x46DC40) */
#define APP_TARGET_FPS_OFFSET  0x16C   /* int32, default 100 — PHYSICS tick rate */
#define APP_RENDER_FPS_OFFSET  0x170   /* int32, default 75  — RENDER frame rate */

/* Address of the render-skip JBE in App_Run (0x46BF55) */
#define RENDER_SKIP_JBE_ADDR   0x0046BF55
#define RENDER_SKIP_JBE_BYTES  2       /* 76 5D = JBE +0x5D */

/* App_Ctor address — we hook AFTER App_Run starts to ensure App is fully initialized */
#define APP_RUN_ADDR 0x0046BD80

/* v2: Only unlock RENDER FPS. Keep PHYSICS (target_fps) at original 100Hz
 * to prevent ball rotation speed-up. Ball_Update uses fixed-delta rotation
 * increments, so increasing the tick rate directly speeds up rotation. */
#define NEW_TARGET_FPS  100    /* KEEP ORIGINAL — physics stays at 100Hz */
#define NEW_RENDER_FPS  1000   /* Unlock render rate to 1000 FPS max */

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

    /* ── Patch 1: App+0x16C (target FPS) — KEEP AT 100 ──────────── */
    /* v2 FIX: Do NOT change target_fps. Ball_Update advances rotation
     * by a fixed increment per tick (no dt scaling). Setting target_fps
     * to 1000 caused the ball to spin 10x faster.
     * We only unlock render_fps below. */

    DWORD appAddr = 0;
    DWORD *pAppPtr = (DWORD *)(0x005341E0 + offset);
    if (!IsBadReadPtr(pAppPtr, 4)) {
        appAddr = *pAppPtr;
    }

    DWORD oldProtect;
    if (appAddr != 0 && !IsBadWritePtr((void *)appAddr, 0x200)) {
        /* v2: Do NOT modify target_fps — leave it at the original 100Hz */
        DWORD *pRenderFPS = (DWORD *)(appAddr + APP_RENDER_FPS_OFFSET);

        /* Only unlock render_fps */
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
            fprintf(f, "Hamsterball FPS Unlock v2.0 — Fixed Rotation\n");
            fprintf(f, "=============================================\n");
            fprintf(f, "Module base: 0x%08lX (offset 0x%08lX)\n", (unsigned long)base, (unsigned long)offset);
            fprintf(f, "App pointer: 0x%08lX\n", (unsigned long)appAddr);
            fprintf(f, "Target FPS: 100 (UNCHANGED — physics stays 100Hz)\n");
            fprintf(f, "Render FPS: 75 -> %d (App+0x170)\n", NEW_RENDER_FPS);
            fprintf(f, "Render-skip JBE at 0x%08lX: NOP'd\n", (unsigned long)(RENDER_SKIP_JBE_ADDR + offset));
            fprintf(f, "Status: ACTIVE — render cap removed, physics preserved\n");
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
