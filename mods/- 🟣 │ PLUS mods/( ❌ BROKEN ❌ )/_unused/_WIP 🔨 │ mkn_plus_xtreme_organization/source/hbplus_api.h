/*
 * hbplus_api.h — Manual IModAPI vtable dispatch for MinGW-compiled HB+ mods.
 *
 * HB+ v2.0/v2.1 IModAPI vtable indices (verified from v2.1 header):
 *   0  = ~IModAPI (deleting destructor)
 *   1  = RegisterCustomHook
 *   10 = CreateToggleButton
 *   11 = CreateSlider
 *   12 = CreateCycleOption
 *   13 = CreateSubmenu
 *   14-18 = RegisterConfig*
 *   19-23 = GetConfig*
 *   24 = PatchMemory
 *   25 = UnlockAll
 *   26 = LockAll
 *   27 = QuitGame
 *   28 = SaveConfig
 *   29 = GetButtonState
 *   30 = GetSliderState
 *   31 = GetCycleOptionState
 *   32 = GetPlayer
 *   38 = GetScene
 *   40 = GetApp
 *   41 = GetGameBaseAddress
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

    void CreateSubmenu(const CustomSubmenu& submenu) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, const void*);
        ((fn_t)hbplus_vtable(ptr, 13))(ptr, &submenu);
    }

    void PatchMemory(DWORD address, const char* bytes, size_t size) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, DWORD, const char*, size_t);
        ((fn_t)hbplus_vtable(ptr, 24))(ptr, address, bytes, size);
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

    DWORD GetGameBaseAddress() {
        typedef DWORD (__attribute__((thiscall)) *fn_t)(void*);
        return ((fn_t)hbplus_vtable(ptr, 41))(ptr);
    }

    bool GetButtonState(const char* id) {
        typedef int (__attribute__((thiscall)) *fn_t)(void*, const char*);
        return ((fn_t)hbplus_vtable(ptr, 29))(ptr, id) != 0;
    }

    float GetSliderState(const char* id) {
        typedef float (__attribute__((thiscall)) *fn_t)(void*, const char*);
        return ((fn_t)hbplus_vtable(ptr, 30))(ptr, id);
    }
};

#endif // HBPLUS_API_H
