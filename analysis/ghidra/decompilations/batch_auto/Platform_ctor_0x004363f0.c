/*
 * Function: Platform_ctor (actually a Rotator subclass — vtable mismatch)
 * Address: 0x004363F0
 * Signature: void * __thiscall Platform_ctor(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, void *param_5)
 * Parameters:
 *   this: Platform* (inherits Rotator) — the platform/rotator object being constructed
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: undefined4 — stored at this+0x10D4 (position X)
 *   param_3: undefined4 — stored at this+0x10D8 (position Y)
 *   param_4: undefined4 — stored at this+0x10DC (position Z)
 *   param_5: void* — passed to Stands_ctor (parent scene for MeshWorld access via +0x878→+0x5C8)
 *
 * Description:
 * Constructor for Platform objects. Despite the name "Platform_ctor", the vtable
 * is set to PTR_Rotator_DeletingDtor (0x4D56A8) — Platform inherits from Rotator,
 * or "Platform" is a specific type of Rotator obstacle.
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_5) — base class init
 *   2. Sets vtable to PTR_Rotator_DeletingDtor (0x4D56A8)
 *   3. Stores position at +0x10D4/+0x10D8/+0x10DC, parent Scene at +0x10D0
 *   4. Creates CollisionLevel (0x10D0 bytes) at +0x10E0
 *   5. Copies +0x434 to CollisionLevel+0x434, clears CollisionLevel+0x431
 *   6. Sets +0x10E4 = 0x42340000 (float 45.0 — platform width or rotation offset)
 *   7. Sets +0x10E8 = 0xBF800000 (float -1.0 — rotation direction or initial angle)
 *   8. Creates a temporary Timer, calls Gfx_SetPosition with offset position,
 *      calls SceneObject_CallUpdate and SceneObject_CallRender
 *   9. Sets +0x10EC = 0 (inactive state), cleans up Timer
 *
 * Name verification: Vtable PTR_Rotator_DeletingDtor confirms Platform is a Rotator
 * subclass. Called from NeonCollisionEvents (0x410B96, 0x410BF8) and CreateWobbly1
 * (0x415B02, 0x415B55, 0x415C1B) — these are obstacle factory functions.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4/+0x10D8/+0x10DC: position (X/Y/Z)
 *   +0x10E0: CollisionLevel pointer (owned)
 *   +0x10E4: width/offset (45.0f)
 *   +0x10E8: rotation direction (-1.0f)
 *   +0x10EC: active state (0 = inactive)
 *
 * Cross-references:
 *   2 calls from NeonCollisionEvents (0x410B96, 0x410BF8)
 *   3 calls from CreateWobbly1 (0x415B02, 0x415B55, 0x415C1B)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
