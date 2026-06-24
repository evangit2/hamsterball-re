/*
 * Hamsterball Collision Hook DLL
 *
 * Hooks three collision dispatch functions:
 *   1. DispatchCollisionEvents          (0x0040C5D0) — shared base handler (all events)
 *   2. TowerCollisionEvents   (0x0040DCD0) — Tower board events
 *   3. ExpertCollisionEvents   (0x0040E6A0) — Expert board events
 *
 * Build: i686-w64-mingw32-gcc -shared -o collision_hook.dll collision_hook.c \
 *          -Wl,--enable-stdcall-fixup
 *
 * MinGW __thiscall workaround: use __fastcall with an extra dummy EDX param.
 * __thiscall(this, arg1, arg2) == __fastcall(this, dummy_edx, arg1, arg2)
 * Both use ECX for first arg, callee cleans 8 bytes of stack.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Constants ────────────────────────────────────────────────────────── */

#define GAME_BASE        0x00400000
#define ADDR_DispatchCollisionEvents          (GAME_BASE + 0x0000C5D0)
#define ADDR_TowerCollisionEvents   (GAME_BASE + 0x0000DCD0)
#define ADDR_ExpertCollisionEvents   (GAME_BASE + 0x0000E6A0)

#define LOG_FILE "collision_log.csv"
#define CFG_FILE "collision_hook.cfg"
#define TRAMP_SIZE 16

/* ── Types ────────────────────────────────────────────────────────────── */

/* __thiscall workaround: __fastcall with dummy EDX arg.
 * this=ECX, dummy=EDX (unused), ball/collObj on stack, callee cleans 8. */
typedef void (__fastcall *handler_t)(void *this_, void *edx_dummy, void *ball, void *collObj);

/* ── Globals ─────────────────────────────────────────────────────────── */

static FILE *g_log = NULL;
static CRITICAL_SECTION g_log_lock;

static handler_t g_orig_DispatchCollisionEvents = NULL;
static handler_t g_orig_Level = NULL;
static handler_t g_orig_Arena = NULL;

static unsigned char g_tramp_DispatchCollisionEvents[TRAMP_SIZE];
static unsigned char g_tramp_Level[TRAMP_SIZE];
static unsigned char g_tramp_Arena[TRAMP_SIZE];

static int g_hook_DispatchCollisionEvents = 1;
static int g_hook_Level = 1;
static int g_hook_Arena = 1;
static int g_log_event_names = 1;
static int g_log_ball_pos = 1;
static int g_log_timestamps = 1;
static char g_filter[256] = "";

/* ── Config ──────────────────────────────────────────────────────────── */

static void load_config(void) {
    FILE *f = fopen(CFG_FILE, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '#' || line[0] == 0) continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line, *val = eq + 1;
        if (!strcmp(key, "hook_DispatchCollisionEvents")) g_hook_DispatchCollisionEvents = atoi(val);
        else if (!strcmp(key, "hook_Level")) g_hook_Level = atoi(val);
        else if (!strcmp(key, "hook_Arena")) g_hook_Arena = atoi(val);
        else if (!strcmp(key, "log_event_names")) g_log_event_names = atoi(val);
        else if (!strcmp(key, "log_ball_pos")) g_log_ball_pos = atoi(val);
        else if (!strcmp(key, "log_timestamps")) g_log_timestamps = atoi(val);
        else if (!strcmp(key, "filter")) {
            strncpy(g_filter, val, sizeof(g_filter)-1);
            g_filter[sizeof(g_filter)-1] = 0;
        }
    }
    fclose(f);
}

/* ── Safe memory helpers (replace SEH) ───────────────────────────────── */

static const char *get_event_name(void *collObj) {
    if (!collObj) return "(null)";
    if (IsBadReadPtr(collObj, 12)) return "(bad-ptr)";
    int *pair = (int *)collObj;
    int meshNode = pair[1];
    if (!meshNode || IsBadReadPtr((void *)meshNode, 0x868)) return "(bad-mesh)";
    int namePtr = *(int *)(meshNode + 0x864);
    if (!namePtr || IsBadReadPtr((void *)namePtr, 1)) return "(bad-name)";
    return (const char *)namePtr;
}

static void get_ball_pos(void *ball, float *x, float *y, float *z) {
    *x = *y = *z = 0.0f;
    if (!ball || IsBadReadPtr(ball, 0x170)) return;
    *x = *(float *)((char *)ball + 0x164);
    *y = *(float *)((char *)ball + 0x168);
    *z = *(float *)((char *)ball + 0x16C);
}

/* ── Logging ──────────────────────────────────────────────────────────── */

static void log_event(const char *handler, void *scene, void *ball, void *collObj) {
    if (!g_log) return;
    const char *eventName = g_log_event_names ? get_event_name(collObj) : "";

    if (g_filter[0]) {
        /* Case-insensitive substring search */
        if (!strstr(eventName, g_filter)) return;
    }

    float x = 0, y = 0, z = 0;
    if (g_log_ball_pos) get_ball_pos(ball, &x, &y, &z);
    DWORD ms = g_log_timestamps ? GetTickCount() : 0;

    EnterCriticalSection(&g_log_lock);
    fprintf(g_log, "%lu,%s,0x%08X,0x%08X,0x%08X,%s,%.2f,%.2f,%.2f\n",
            (unsigned long)ms, handler,
            (unsigned int)(uintptr_t)scene,
            (unsigned int)(uintptr_t)ball,
            (unsigned int)(uintptr_t)collObj,
            eventName, x, y, z);
    fflush(g_log);
    LeaveCriticalSection(&g_log_lock);
}

/* ── Hook callbacks (__fastcall = __thiscall with dummy EDX) ──────────── */

void __fastcall hook_DispatchCollisionEvents(void *this_, void *edx_dummy,
                                    void *ball, void *collObj) {
    (void)edx_dummy;
    log_event("DispatchCollisionEvents", this_, ball, collObj);
    if (g_orig_DispatchCollisionEvents)
        g_orig_DispatchCollisionEvents(this_, NULL, ball, collObj);
}

void __fastcall hook_TowerCollisionEvents(void *this_, void *edx_dummy,
                                             void *ball, void *collObj) {
    (void)edx_dummy;
    log_event("TowerCollisionEvents", this_, ball, collObj);
    if (g_orig_Level)
        g_orig_Level(this_, NULL, ball, collObj);
}

void __fastcall hook_ExpertCollisionEvents(void *this_, void *edx_dummy,
                                            void *ball, void *collObj) {
    (void)edx_dummy;
    log_event("ExpertCollisionEvents", this_, ball, collObj);
    if (g_orig_Arena)
        g_orig_Arena(this_, NULL, ball, collObj);
}

/* ── Inline hook engine ──────────────────────────────────────────────── */

static int install_hook(void *target, void *hook, unsigned char *trampoline) {
    DWORD oldProtect;
    unsigned char *t = (unsigned char *)target;

    if (!VirtualProtect(t, TRAMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;

    /* Copy original bytes to trampoline */
    memcpy(trampoline, t, TRAMP_SIZE);

    /* Trampoline: original 5 bytes + jmp back to target+5 */
    trampoline[5] = 0xE9;
    *(unsigned long *)(trampoline + 6) =
        (unsigned long)((char *)target + 5 - (char *)(trampoline + 5) - 5);

    /* Make trampoline executable */
    DWORD tp;
    VirtualProtect(trampoline, TRAMP_SIZE, PAGE_EXECUTE_READWRITE, &tp);

    /* Overwrite target: jmp rel32 */
    unsigned long rel = (unsigned long)((char *)hook - (char *)target - 5);
    t[0] = 0xE9;
    *(unsigned long *)(t + 1) = rel;

    FlushInstructionCache(GetCurrentProcess(), target, 5);
    return 1;
}

static int uninstall_hook(void *target, unsigned char *trampoline) {
    DWORD oldProtect;
    if (!VirtualProtect(target, TRAMP_SIZE, PAGE_READWRITE, &oldProtect))
        return 0;
    memcpy(target, trampoline, 5);
    VirtualProtect(target, TRAMP_SIZE, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), target, 5);
    return 1;
}

/* ── Init thread ─────────────────────────────────────────────────────── */

static DWORD WINAPI hook_thread(LPVOID lpParam) {
    (void)lpParam;
    Sleep(2000);
    load_config();

    g_log = fopen(LOG_FILE, "w");
    if (g_log) {
        fprintf(g_log, "timestamp_ms,handler,scene_ptr,ball_ptr,collobj_ptr,event_name,x,y,z\n");
        fflush(g_log);
    }
    InitializeCriticalSection(&g_log_lock);

    HMODULE hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)(uintptr_t)hExe;
    DWORD offset = base - GAME_BASE;

    void *pDispatchCollisionEvents = (void *)(ADDR_DispatchCollisionEvents + offset);
    void *pLevel = (void *)(ADDR_TowerCollisionEvents + offset);
    void *pArena = (void *)(ADDR_ExpertCollisionEvents + offset);

    if (g_hook_DispatchCollisionEvents && install_hook(pDispatchCollisionEvents, hook_DispatchCollisionEvents, g_tramp_DispatchCollisionEvents)) {
        g_orig_DispatchCollisionEvents = (handler_t)g_tramp_DispatchCollisionEvents;
        if (g_log) { fprintf(g_log, "# Hooked DispatchCollisionEvents at 0x%08X\n", (unsigned int)(uintptr_t)pDispatchCollisionEvents); }
    }
    if (g_hook_Level && install_hook(pLevel, hook_TowerCollisionEvents, g_tramp_Level)) {
        g_orig_Level = (handler_t)g_tramp_Level;
        if (g_log) { fprintf(g_log, "# Hooked TowerCollisionEvents at 0x%08X\n", (unsigned int)(uintptr_t)pLevel); }
    }
    if (g_hook_Arena && install_hook(pArena, hook_ExpertCollisionEvents, g_tramp_Arena)) {
        g_orig_Arena = (handler_t)g_tramp_Arena;
        if (g_log) { fprintf(g_log, "# Hooked ExpertCollisionEvents at 0x%08X\n", (unsigned int)(uintptr_t)pArena); }
    }

    if (g_log) { fprintf(g_log, "# Hooks installed. Logging active.\n"); fflush(g_log); }
    return 0;
}

/* ── DLL Entry ───────────────────────────────────────────────────────── */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, hook_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        {
            HMODULE hExe = GetModuleHandleA(NULL);
            DWORD base = (DWORD)(uintptr_t)hExe;
            DWORD offset = base - GAME_BASE;
            if (g_hook_DispatchCollisionEvents && g_orig_DispatchCollisionEvents)
                uninstall_hook((void *)(ADDR_DispatchCollisionEvents + offset), g_tramp_DispatchCollisionEvents);
            if (g_hook_Level && g_orig_Level)
                uninstall_hook((void *)(ADDR_TowerCollisionEvents + offset), g_tramp_Level);
            if (g_hook_Arena && g_orig_Arena)
                uninstall_hook((void *)(ADDR_ExpertCollisionEvents + offset), g_tramp_Arena);
            if (g_log) {
                fprintf(g_log, "# Hooks uninstalled. DLL detaching.\n");
                fclose(g_log);
            }
            DeleteCriticalSection(&g_log_lock);
        }
        break;
    }
    return TRUE;
}

/* Exported for manual injection */
__declspec(dllexport) void __cdecl InitHooks(void) { /* done in DllMain */ }
