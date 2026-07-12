// hbplus_api.h — MinGW-compatible wrapper for HB+ IModAPI
// 
// Problem: MinGW adds an extra destructor slot to C++ vtables, shifting all
// virtual function offsets by 1. When calling api->CreateToggleButton(),
// MinGW generates a call to the wrong vtable slot, hitting CreateSlider()
// instead. This affects ALL IModAPI method calls.
//
// Solution: Don't use C++ virtual dispatch. Instead, call IModAPI methods
// by directly indexing the vtable with the correct VS offsets.
//
// Usage: Replace `api->Method(args)` with `API(api)->Method(args)`.
// The API() macro wraps the IModAPI pointer in a struct that uses
// manual vtable indexing.

#ifndef HBPLUS_API_H
#define HBPLUS_API_H

#include "HamsterballAPI.h"

// Helper: call a __thiscall method via vtable
// VS vtable layout: [0]=dtor, [1]=first virtual, [2]=second, etc.
// MinGW would use [0]=thunk, [1]=dtor, [2]=first virtual — shifted by 1.
// We bypass this by manually reading the vtable pointer and indexing directly.

// Get vtable entry N from an object (0-indexed, VS layout)
static inline void* hbplus_vtable(void* obj, int index) {
    void** vtable = *(void***)obj;
    return vtable[index];
}

// Call __thiscall with 1 arg
static inline void hbplus_call1(void* obj, int vtableIdx, void* arg1) {
    typedef void (__attribute__((thiscall)) *fn_t)(void*, void*);
    ((fn_t)hbplus_vtable(obj, vtableIdx))(obj, arg1);
}

// Call __thiscall with 2 args
static inline void hbplus_call2(void* obj, int vtableIdx, void* arg1, void* arg2) {
    typedef void (__attribute__((thiscall)) *fn_t)(void*, void*, void*);
    ((fn_t)hbplus_vtable(obj, vtableIdx))(obj, arg1, arg2);
}

// Call __thiscall with 0 args, returns void*
static inline void* hbplus_call0_ptr(void* obj, int vtableIdx) {
    typedef void* (__attribute__((thiscall)) *fn_t)(void*);
    return ((fn_t)hbplus_vtable(obj, vtableIdx))(obj);
}

// Call __thiscall with 0 args, returns int
static inline int hbplus_call0_int(void* obj, int vtableIdx) {
    typedef int (__attribute__((thiscall)) *fn_t)(void*);
    return ((fn_t)hbplus_vtable(obj, vtableIdx))(obj);
}

// Call __thiscall with 0 args, returns float
static inline float hbplus_call0_float(void* obj, int vtableIdx) {
    typedef float (__attribute__((thiscall)) *fn_t)(void*);
    return ((fn_t)hbplus_vtable(obj, vtableIdx))(obj);
}

// Call __thiscall with 1 arg, returns bool
static inline bool hbplus_call1_bool(void* obj, int vtableIdx, int arg1) {
    typedef bool (__attribute__((thiscall)) *fn_t)(void*, int);
    return ((fn_t)hbplus_vtable(obj, vtableIdx))(obj, arg1);
}

// Call __thiscall with 1 arg, returns bool (char* arg)
static inline bool hbplus_call1_bool_str(void* obj, int vtableIdx, const char* arg1) {
    typedef bool (__attribute__((thiscall)) *fn_t)(void*, const char*);
    return ((fn_t)hbplus_vtable(obj, vtableIdx))(obj, arg1);
}

// Call __thiscall with 1 arg, returns float (char* arg)
static inline float hbplus_call1_float_str(void* obj, int vtableIdx, const char* arg1) {
    typedef float (__attribute__((thiscall)) *fn_t)(void*, const char*);
    return ((fn_t)hbplus_vtable(obj, vtableIdx))(obj, arg1);
}

// ── VS vtable indices for IModAPI ──────────────────────────────────
// These are the CORRECT indices for VS-compiled IModAPI objects.
// MinGW would add 1 to each, but we bypass that with manual indexing.
#define HBPLUS_VT_CreateToggleButton  10
#define HBPLUS_VT_CreateSlider        11
#define HBPLUS_VT_GetButtonState      21
#define HBPLUS_VT_GetSliderState      22
#define HBPLUS_VT_GetPlayer           23
#define HBPLUS_VT_GetPlayer2          24
#define HBPLUS_VT_GetPlayer3          25
#define HBPLUS_VT_GetPlayer4          26
#define HBPLUS_VT_GetScene            29
#define HBPLUS_VT_GetApp              31
#define HBPLUS_VT_ApplyForce          19
#define HBPLUS_VT_PatchMemory          13
#define HBPLUS_VT_RegisterCustomHook   1
#define HBPLUS_VT_IsKeyDown            4
#define HBPLUS_VT_WasKeyPressed       5
#define HBPLUS_VT_WasKeyReleased      6
#define HBPLUS_VT_DrawCustomText      40  // Adjust based on actual count
#define HBPLUS_VT_GetTimerTime        37

// ── Wrapper struct ─────────────────────────────────────────────────
// Wrap IModAPI* in a struct that provides inline methods using manual vtable indexing.
struct HBPlusAPI {
    void* ptr; // IModAPI*

    void CreateToggleButton(const CustomButton& button, void* modInstance) {
        hbplus_call2(ptr, HBPLUS_VT_CreateToggleButton, (void*)&button, modInstance);
    }
    void CreateSlider(const CustomSlider& slider, void* modInstance) {
        hbplus_call2(ptr, HBPLUS_VT_CreateSlider, (void*)&slider, modInstance);
    }
    bool GetButtonState(const char* id) {
        return hbplus_call1_bool_str(ptr, HBPLUS_VT_GetButtonState, id);
    }
    float GetSliderState(const char* id) {
        return hbplus_call1_float_str(ptr, HBPLUS_VT_GetSliderState, id);
    }
    void* GetPlayer() { return hbplus_call0_ptr(ptr, HBPLUS_VT_GetPlayer); }
    void* GetScene() { return hbplus_call0_ptr(ptr, HBPLUS_VT_GetScene); }
    void* GetApp() { return hbplus_call0_ptr(ptr, HBPLUS_VT_GetApp); }
    bool IsKeyDown(int dik) { return hbplus_call1_bool(ptr, HBPLUS_VT_IsKeyDown, dik); }
    bool WasKeyPressed(int dik) { return hbplus_call1_bool(ptr, HBPLUS_VT_WasKeyPressed, dik); }
    void ApplyForce(void* ball, float x, float y, float z, float mag) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, void*, float, float, float, float);
        ((fn_t)hbplus_vtable(ptr, HBPLUS_VT_ApplyForce))(ptr, ball, x, y, z, mag);
    }
    void PatchMemory(DWORD addr, const char* bytes, size_t size) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, DWORD, const char*, size_t);
        ((fn_t)hbplus_vtable(ptr, HBPLUS_VT_PatchMemory))(ptr, addr, bytes, size);
    }
    void RegisterCustomHook(DWORD addr, void* hook, void** orig) {
        typedef void (__attribute__((thiscall)) *fn_t)(void*, DWORD, void*, void**);
        ((fn_t)hbplus_vtable(ptr, HBPLUS_VT_RegisterCustomHook))(ptr, addr, hook, orig);
    }
};

// Convenience macro: API(api) creates an HBPlusAPI wrapper
#define HBAPI(api) HBPlusAPI{api}

#endif // HBPLUS_API_H
