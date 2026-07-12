/*
 * LocalGravity_MinGW.cpp — Minimal test: override gravity_scale (ball+0x278)
 *
 * Always on, no UI button. Writes 0.125f to ball+0x278 (gravity_scale, default 0.5f)
 * and copies the same value to physics+0x1C0 (gravity Y accumulator).
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.0.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Local Gravity"; }
static const char* __thiscall get_author(void*) { return "BookwormKevin"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    /* No UI — always on */
}

static void __thiscall ball_update_impl(void* thisptr, void* ball) {
    if (!ball) return;

    Ball* b = (Ball*)ball;
    PhysicsObject* phys = b->physics_object;
    if (!phys) return;

    /* Write 0.125f to ball+0x278 (gravity_scale, default 0.5f) */
    *(float*)((char*)ball + 0x278) = 0.125f;

    /* Copy same value to physics+0x1C0 (gravity Y accumulator) */
    *(float*)((char*)phys + 0x1C0) = 0.125f;
}

/* No-op implementations for unused callbacks */
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall game_update_impl(void*) {}
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
