/*
 * hbplus_api.h — Manual IModAPI vtable dispatch for MinGW-compiled HB+ mods.
 *
 * HB+ v2.0 added new IModAPI methods (CreateCycleOption, CreateSubmenu,
 * RegisterConfig*, GetConfig*, GetCycleOptionState) which shifted ALL
 * vtable indices after CreateSlider by +8, and indices after GetSliderState
 * by +1 (GetCycleOptionState).
 *
 * HB+ v2.0/v2.1 IModAPI vtable indices (verified from v2.1 header):
 *   0  = ~IModAPI (deleting destructor)
 *   1  = RegisterCustomHook
 *   10 = CreateToggleButton
 *   11 = CreateSlider
 *   29 = GetButtonState
 *   30 = GetSliderState
 *   32 = GetPlayer
 *   38 = GetScene
 *   40 = GetApp
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
        return (Ball*)((fn_t)hbplus_vtable(ptr, 32))(ptr);
    }

    Scene* GetScene() {
        typedef void* (__attribute__((thiscall)) *fn_t)(void*);
        return (Scene*)((fn_t)hbplus_vtable(ptr, 38))(ptr);
    }

    App* GetApp() {
        typedef void* (__attribute__((thiscall)) *fn_t)(void*);
        return (App*)((fn_t)hbplus_vtable(ptr, 40))(ptr);
    }

    bool GetButtonState(const char* id) {
        typedef int (__attribute__((thiscall)) *fn_t)(void*, const char*);
        return ((fn_t)hbplus_vtable(ptr, 29))(ptr, id) != 0;
    }

    float GetSliderState(const char* id) {
        typedef float (__attribute__((thiscall)) *fn_t)(void*, const char*);
        return ((fn_t)hbplus_vtable(ptr, 30))(ptr, id);
    }

    void RegisterCustomHook(DWORD addr, void* hook, void** orig) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, DWORD, void*, void**);
        ((fn_t)hbplus_vtable(ptr, 1))(ptr, addr, hook, orig);
    }
};

// Convenience macro: HBAPI(api) creates an HBPlusAPI wrapper
#define HBAPI(api) HBPlusAPI{api}

#endif // HBPLUS_API_H
