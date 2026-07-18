// WidescreenUIFix_MinGW.cpp — MinGW build with manual vtable + nocrt
// Hooks Gfx_TransformX to fix UI stretching in widescreen modes.
// Updated for HB+ v2.1 (17-entry vtable, API_VERSION 3).

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

static bool g_enabled = true;

typedef float (__fastcall *TransformX_t)(void*, void*, float);
static TransformX_t orig_TransformX = NULL;

typedef void (__fastcall *DrawScreenRect_t)(void*, void*, int, int, int, int);
static DrawScreenRect_t orig_DrawScreenRect = NULL;

static bool getWidescreenParams(void* gfx, float* outScale, float* outMargin) {
    DWORD gfxAddr = (DWORD)gfx;
    if (IsBadReadPtr(gfx, 0x800)) return false;
    DWORD config = *(DWORD*)(gfxAddr + 0x5c);
    if (!config || IsBadReadPtr((void*)config, 0x200)) return false;
    DWORD bbWidth = *(DWORD*)(config + 0x15c);
    DWORD bbHeight = *(DWORD*)(config + 0x160);
    if (bbWidth <= 0 || bbHeight <= 0) return false;
    float aspect = (float)bbWidth / (float)bbHeight;
    if (aspect <= 1.34f) return false;
    float ratio43 = 4.0f / 3.0f;
    *outScale = ratio43 / aspect;
    *outMargin = ((float)bbWidth - (float)bbHeight * ratio43) / 2.0f;
    return true;
}

static float __fastcall hook_TransformX(void* gfx, void* edx, float pixel_x) {
    float result = orig_TransformX(gfx, edx, pixel_x);
    if (!g_enabled) return result;
    float scaleFactor, margin;
    if (!getWidescreenParams(gfx, &scaleFactor, &margin)) return result;
    return result * scaleFactor + margin;
}

static int g_rectCount = 0;

static void __fastcall hook_DrawScreenRect(void* gfx, void* edx, int x, int y, int w, int h) {
    if (!g_enabled) { orig_DrawScreenRect(gfx, edx, x, y, w, h); return; }

    float scaleFactor, margin;
    if (!getWidescreenParams(gfx, &scaleFactor, &margin)) {
        orig_DrawScreenRect(gfx, edx, x, y, w, h);
        return;
    }

    DWORD gfxAddr = (DWORD)gfx;
    DWORD config = *(DWORD*)(gfxAddr + 0x5c);

    float* pScaleX = (float*)(config + 0x1f8);
    float origScaleX = *pScaleX;
    float newScaleX = origScaleX * scaleFactor;

    g_rectCount++;

    DWORD oldProtect;
    VirtualProtect(pScaleX, 4, PAGE_READWRITE, &oldProtect);
    *pScaleX = newScaleX;

    int newX = x + (int)(margin / newScaleX);

    orig_DrawScreenRect(gfx, edx, newX, y, w, h);

    *pScaleX = origScaleX;
    VirtualProtect(pScaleX, 4, oldProtect, &oldProtect);
}

// ── Manual vtable (17 entries, HB+ v2.0/v2.1 MSVC layout) ──────────
// Slot 10 = onCycleOptionChange (added in v2.0, unchanged in v2.1)
static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Widescreen UI Fix"; }
static const char* __thiscall get_author(void*) { return "BookwormKevin"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "rsks"; }

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
    HBAPI(api).RegisterCustomHook(0x453e90, (void*)hook_TransformX, (void**)&orig_TransformX);
    HBAPI(api).RegisterCustomHook(0x455d60, (void*)hook_DrawScreenRect, (void**)&orig_DrawScreenRect);
}

static void __thiscall ball_update(void*, void*) {}
static void __thiscall render_apply(void*, void*, float*) {}
static void __thiscall button_toggle(void*, const char* id, bool state) {
    if (nc_strcmp(id, "ws_ui_fix") == 0) g_enabled = state;
}
static void __thiscall slider_change(void*, const char*, float) {}
static void __thiscall cycle_option_change(void*, const char*, const char*) {}  // v2.0+ slot 10
static void __thiscall game_update(void*) {}
static void __thiscall event_collide(void*, void*, const char*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, void*, void*) {}
static void __thiscall scene_end(void*) {}
static void __thiscall level_start(void*) {}

// 17-entry vtable (HB+ v2.0/v2.1)
// Slots 0-9: dtor, GetModName, GetAuthorName, GetApiVersion, GetContributors,
//            Initialize, onBallUpdate, onRenderApply, onButtonToggle, onSliderChange
// Slot 10:   onCycleOptionChange (NEW in v2.0)
// Slots 11-16: onGameUpdate, onEventPlaneCollide, onTextRenderLoop, onBallBump, onSceneEnd, onLevelStart
static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]  ~HamsterballAPI
    (void*)get_mod_name,         // [1]  GetModName
    (void*)get_author,           // [2]  GetAuthorName
    (void*)get_version,          // [3]  GetApiVersion
    (void*)get_contributors,     // [4]  GetContributors
    (void*)init_impl,            // [5]  Initialize
    (void*)ball_update,          // [6]  onBallUpdate
    (void*)render_apply,         // [7]  onRenderApply
    (void*)button_toggle,        // [8]  onButtonToggle
    (void*)slider_change,        // [9]  onSliderChange
    (void*)cycle_option_change,  // [10] onCycleOptionChange (v2.0+)
    (void*)game_update,          // [11] onGameUpdate
    (void*)event_collide,        // [12] onEventPlaneCollide
    (void*)text_render,          // [13] onTextRenderLoop
    (void*)ball_bump,            // [14] onBallBump
    (void*)scene_end,            // [15] onSceneEnd
    (void*)level_start,          // [16] onLevelStart
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
