// Hamsterball - Callback and Function Pointer Typedefs
// Extracted from vtable analysis and decompiler signatures
//
// Ball vtable (0x4CF3A0):
//   [0] Ball_dtor: void* __thiscall dtor(void *this, byte flags)
//   [1] 0x405100 thunk
//   [2] Ball_Render: void __fastcall render(int param_1) [ECX=this]
//   [3] 0x402A70 thunk  
//   [4] 0x408390 thunk
//   [5] 0x401590 thunk
//   [6] Ball_ApplyForceWithMultipliers: void __thiscall applyForce(void *this, float x, float y, float z, float mult)
//   [7] 0x402C10 thunk
//   [8] 0x409480 thunk

#ifndef GAME_TYPEDEFS_H
#define GAME_TYPEDEFS_H

// === Ball vtable method types ===
typedef void* (__thiscall *Ball_dtor_fn)(void *this, byte flags);
typedef void (__thiscall *Ball_render_fn)(void *this);
typedef void (__thiscall *Ball_apply_force_fn)(void *this, float x, float y, float z, float mult);

// === Scene/Board vtable method types ===
typedef void (__thiscall *Scene_dtor_fn)(void *this);
typedef void (__thiscall *Scene_update_fn)(void *this);
typedef void (__thiscall *Scene_render_fn)(void *this);

// === SceneObject vtable method types ===
// vtable at 0x4D934C (5 entries + null sentinel):
//   [0] 0x46B280 - SceneObject_dtors
//   [1] 0x46B490 - SceneObject_thunk
//   [2] 0x46B4B0 - SceneObject_thunk2
//   [3] 0x46B670 - SceneObject_thunk3
//   [4] 0x46B4D0 - SceneObject_thunk4
//   [5] null sentinel (0xFFFFFFE0)
//   [6] 0x46B9F0 - SceneObject_fn6 (probably render)
typedef void (__thiscall *SceneObject_dtor_fn)(void *this, byte flags);
typedef void (__thiscall *SceneObject_update_fn)(void *this);
typedef void (__thiscall *SceneObject_render_fn)(void *this);

// === Gadget vtable method types (at 0x4D9170) ===
// Inherits SceneObject, adds Gadget-specific methods
// vtable at 0x004CF584 (second vtable in Gadget):
typedef void (__thiscall *Gadget_update_fn)(void *this);

// === RumbleBoard vtable (0x4D1358) ===
// Inherits Board vtable at 0x4D0260

// === App vtable method types (from App_Initialize) ===
typedef void (__thiscall *App_init_fn)(void *this, uint32_t hInstance, uint32_t cmdShow);
typedef void (__thiscall *App_vcall_fn)(void *this);  // Generic vtable call with no params

// === AthenaList callback types ===
typedef void (__cdecl *AthenaList_Callback)(void *item);
typedef int (__cdecl *AthenaList_Compare)(const void *a, const void *b);

// === Timer/Render callback ===
typedef void (__thiscall *UITimer_Callback)(void *this);
typedef void (__thiscall *RenderContext_Callback)(void *this);

// === Collision function pointer ===
typedef int (__thiscall *CollisionHandler)(void *this, void *other, int event_type);

// === Key global function pointers ===
// 0x4FD680: g_App (App singleton)
// 0x4CF584: Gadget secondary vtable (SceneObject methods)
// 0x4D934C: SceneObject dtor vtable
// 0x4D9170: SceneObject scalar dtor vtable
// 0x4D0260: Scene/Board vtable
// 0x4D1358: RumbleBoard vtable
// 0x4CF3A0: Ball vtable
// 0x4CF314: GameObject sub2_dtor (initial Ball vtable before full init)

// === Key struct sizes ===
// Ball: 0xC98 bytes (3224)
// Gadget: 0x870 bytes (2160)
// Board: ~0x4368 bytes
// SceneObject: ~0xD4 bytes
// RumbleBoard: ~0x47D4 bytes
// App: ~0xA00 bytes (estimated from WinMain + App_Initialize_Full)
// RumbleBoard timer: 0x14 bytes (20)
// Vec3: 0xC bytes (12)

#endif // GAME_TYPEDEFS_H