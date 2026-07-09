// WidescreenUIFix_MinGW.cpp — MinGW build with manual vtable + nocrt
// Hooks Gfx_TransformX to fix UI stretching in widescreen modes.

#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#include <windows.h>

#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncpy nc_strncpy
#define snprintf nc_snprintf

// ── State ──────────────────────────────────────────────────────────
static bool g_enabled = true;
static bool g_inUIPass = false;
static float g_scaleFactor = 1.0f;
static float g_margin = 0.0f;

// ── Hook typedefs & originals ──────────────────────────────────────
typedef void (__fastcall *SetViewport_t)(void*, void*, int, int);
static SetViewport_t orig_SetViewport = NULL;

typedef void (__fastcall *SceneRender_t)(void*, void*, void*);
static SceneRender_t orig_SceneRender = NULL;

typedef float (__fastcall *TransformY_t)(void*, void*, float);
static TransformY_t orig_TransformY = NULL;

// ── Hook implementations ───────────────────────────────────────────
static float __fastcall hook_TransformY(void* gfx, void* edx, float pixel_x) {
    float result = orig_TransformY(gfx, edx, pixel_x);
    if (!g_enabled || !g_inUIPass) return result;
    return result * g_scaleFactor + g_margin;
}

static void __fastcall hook_SetViewport(void* gfx, void* edx, int param1, int param2) {
    orig_SetViewport(gfx, edx, param1, param2);

    if (!g_enabled) return;
    if (param1 != 0 || param2 != 0) return;

    DWORD appPtr = *(DWORD*)0x5341E0;
    if (!appPtr || IsBadReadPtr((void*)appPtr, 0x300)) return;
    DWORD profile = *(DWORD*)(appPtr + 0x220);
    if (!profile || IsBadReadPtr((void*)(profile + 0x10), 4)) return;
    DWORD board = *(DWORD*)(profile + 0x0C);
    if (!board || IsBadReadPtr((void*)board, 4)) return;

    g_inUIPass = true;

    DWORD gfxAddr = (DWORD)gfx;
    if (IsBadReadPtr(gfx, 0x800)) return;

    DWORD config = *(DWORD*)(gfxAddr + 0x5c);
    if (!config || IsBadReadPtr((void*)config, 0x200)) return;

    DWORD bbWidth = *(DWORD*)(config + 0x15c);
    DWORD bbHeight = *(DWORD*)(config + 0x160);
    if (bbWidth <= 0 || bbHeight <= 0) return;

    float aspect = (float)bbWidth / (float)bbHeight;
    if (aspect <= 1.34f) return;

    float ratio43 = 4.0f / 3.0f;
    g_scaleFactor = ratio43 / aspect;
    g_margin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;
}

static void __fastcall hook_SceneRender(void* this_ptr, void* edx, void* param1) {
    g_inUIPass = false;
    orig_SceneRender(this_ptr, edx, param1);
    g_inUIPass = false;
}

// ── Manual vtable (16 entries, MSVC layout) ────────────────────────
static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Widescreen UI Fix"; }
static const char* __thiscall get_author(void*) { return "BookwormKevin"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return ""; }

static IModAPI* g_api = NULL;
static void* g_modObj = NULL;

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    g_modObj = thisptr;

    CustomButton btn;
    btn.id = "ws_ui_fix";
    btn.displayText = "Widescreen UI Fix";
    btn.defaultState = true;

    HBAPI(api).CreateToggleButton(btn, thisptr);
    HBAPI(api).RegisterCustomHook(0x453e90, (void*)hook_TransformY, (void**)&orig_TransformY);
    HBAPI(api).RegisterCustomHook(0x454f10, (void*)hook_SetViewport, (void**)&orig_SetViewport);
    HBAPI(api).RegisterCustomHook(0x41a2e0, (void*)hook_SceneRender, (void**)&orig_SceneRender);
}

static void __thiscall ball_update(void*, void*) {}
static void __thiscall render_apply(void*, void*, float*) {}
static void __thiscall button_toggle(void*, const char* id, bool state) {
    if (nc_strcmp(id, "ws_ui_fix") == 0) g_enabled = state;
}
static void __thiscall slider_change(void*, const char*, float) {}
static void __thiscall game_update(void*) {
    // Reset every frame so g_inUIPass doesn't persist across frames.
    // In race modes, Scene_Render handles this. But ArenaBoard (Rodent
    // Rumble) doesn't use Scene_Render, so without this reset, once
    // g_inUIPass is set true it stays true forever — leaking into
    // pause menus and post-match screens.
    g_inUIPass = false;
}
static void __thiscall event_collide(void*, void*, const char*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, void*, void*) {}
static void __thiscall scene_end(void*) {}
static void __thiscall level_start(void*) {}

static void* g_vtable[16] = {
    (void*)sc_dtor,
    (void*)get_mod_name,
    (void*)get_author,
    (void*)get_version,
    (void*)get_contributors,
    (void*)init_impl,
    (void*)ball_update,
    (void*)render_apply,
    (void*)button_toggle,
    (void*)slider_change,
    (void*)game_update,
    (void*)event_collide,
    (void*)text_render,
    (void*)ball_bump,
    (void*)scene_end,
    (void*)level_start,
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
