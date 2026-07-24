/*
 * hbplus_api.h - Manual IModAPI vtable dispatch for MinGW-compiled HB+ mods.
 * HB+ v2.0/v2.1 IModAPI vtable indices.
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

    void PlaySoundEffect(void* soundEffect, float volume) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, void*, float);
        ((fn_t)hbplus_vtable(ptr, 49))(ptr, soundEffect, volume);
    }

    void Play3dSoundEffect(void* soundEffect, Vec3 position, float volume) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, void*, Vec3, float);
        ((fn_t)hbplus_vtable(ptr, 50))(ptr, soundEffect, position, volume);
    }
};

#endif // HBPLUS_API_H
