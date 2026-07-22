/*
 * KeyboardForce_MinGW.cpp — MinGW cross-compile version of Keyboard Force mod.
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

static constexpr DWORD KB_FORCE_RVA = 0xD03B8;
static constexpr float KB_FORCE_DEFAULT = 0.12f;
static constexpr float KB_FORCE_MIN = 0.10f;
static constexpr float KB_FORCE_MAX = 0.26f;

static void* g_storedApi = NULL;

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Keyboard Force"; }
static const char* __thiscall get_author(void*) { return "BookwormKevin"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;

    HBPlusAPI hb = { modApi };
    CustomSlider slider;
    slider.id = "KB_FORCE";
    slider.displayText = "Keyboard/Controller Force";
    slider.startingValue = KB_FORCE_DEFAULT;
    slider.stepSize = 0.01f;
    slider.lowerBound = KB_FORCE_MIN;
    slider.upperBound = KB_FORCE_MAX;
    slider.decimalPlaces = 2;
    slider.unitName = "";
    slider.color = Color(0.4f, 0.8f, 1.0f, 1.0f);
    slider.submenuID = "MAIN";
    slider.maxShiftMult = 10;
    hb.CreateSlider(slider, thisptr);
}

static void __thiscall game_update_impl(void* thisptr) {
    if (!g_storedApi) return;

    HBPlusAPI hb = { g_storedApi };
    float value = hb.GetSliderState("KB_FORCE");
    if (value < KB_FORCE_MIN) value = KB_FORCE_MIN;
    if (value > KB_FORCE_MAX) value = KB_FORCE_MAX;

    DWORD gameBase = (DWORD)GetModuleHandleA(NULL);
    float* pForceScale = (float*)(gameBase + KB_FORCE_RVA);
    *pForceScale = value;
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10]
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
