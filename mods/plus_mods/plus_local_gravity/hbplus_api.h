/*
 * hbplus_api.h — Manual IModAPI vtable dispatch for MinGW-compiled HB+ mods.
 *
 * MinGW and MSVC have different vtable layouts for virtual destructors,
 * so api->Method() calls hit the wrong vtable slot. This wrapper manually
 * indexes the IModAPI vtable with correct VS offsets.
 *
 * VS IModAPI vtable indices (verified on Wine with real HB+ framework):
 *   0  = ~IModAPI (deleting destructor)
 *   1  = RegisterCustomHook
 *   10 = CreateToggleButton
 *   11 = CreateSlider
 *   21 = GetPlayer
 *   29 = GetScene
 *   31 = GetApp
 *   35 = GetButtonState
 *   36 = GetSliderState
 */
#ifndef HBPLUS_API_H
#define HBPLUS_API_H

#include "HamsterballAPI.h"

static inline void* hbplus_vtable(void* obj, int index) {
    void** vtable = *(void***)obj;
    return vtable[index];
}

struct HBPlusAPI {
    void* ptr;

    void CreateToggleButton(const CustomButton& btn, void* modInstance) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, const void*, void*);
        ((fn_t)hbplus_vtable(ptr, 10))(ptr, &btn, modInstance);
    }

    void CreateSlider(const CustomSlider& slider, void* modInstance) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, const void*, void*);
        ((fn_t)hbplus_vtable(ptr, 11))(ptr, &slider, modInstance);
    }

    Ball* GetPlayer() {
        typedef void* (__attribute__((thiscall)) *fn_t)(void*);
        return (Ball*)((fn_t)hbplus_vtable(ptr, 21))(ptr);
    }

    Scene* GetScene() {
        typedef void* (__attribute__((thiscall)) *fn_t)(void*);
        return (Scene*)((fn_t)hbplus_vtable(ptr, 29))(ptr);
    }

    App* GetApp() {
        typedef void* (__attribute__((thiscall)) *fn_t)(void*);
        return (App*)((fn_t)hbplus_vtable(ptr, 31))(ptr);
    }

    bool GetButtonState(const char* id) {
        typedef int (__attribute__((thiscall)) *fn_t)(void*, const char*);
        return ((fn_t)hbplus_vtable(ptr, 35))(ptr, id) != 0;
    }

    float GetSliderState(const char* id) {
        typedef float (__attribute__((thiscall)) *fn_t)(void*, const char*);
        return ((fn_t)hbplus_vtable(ptr, 36))(ptr, id);
    }
};

#endif // HBPLUS_API_H
