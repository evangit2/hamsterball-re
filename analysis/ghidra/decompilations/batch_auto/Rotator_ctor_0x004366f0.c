/*
 * Function: Rotator_ctor
 * Address: 0x004366F0
 * Signature: void * __thiscall Rotator_ctor(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5, void *param_6)
 * Parameters:
 *   this: Rotator* — the rotator object being constructed
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: undefined4 — stored at this+0x10D4 (position X)
 *   param_3: undefined4 — stored at this+0x10D8 (position Y)
 *   param_4: undefined4 — stored at this+0x10DC (position Z)
 *   param_5: undefined4 — stored at this+0x10E0 (rotation speed or extra parameter)
 *   param_6: void* — passed to Stands_ctor (parent scene for MeshWorld access)
 *
 * Description:
 * Constructor for Rotator objects (rotating platform obstacles). Uses vtable
 * PTR_Rotator_DeletingDtor2 (0x4D5708) — a DIFFERENT vtable from Platform_ctor
 * (which uses 0x4D56A8). This is the "full" Rotator constructor with 6 params.
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_6) — base class init
 *   2. Sets vtable to PTR_Rotator_DeletingDtor2 (0x4D5708)
 *   3. Stores position at +0x10D4/+0x10D8/+0x10DC, parent Scene at +0x10D0
 *   4. Stores param_5 at +0x10E0 (rotation speed or extra data)
 *   5. Creates CollisionLevel (0x10D0 bytes) at +0x10F0
 *   6. Copies +0x434 to CollisionLevel+0x434, clears CollisionLevel+0x431
 *   7. Sets +0x10ED = 0 (sound started flag = false)
 *   8. Sets +0x10E4 = 0 (current rotation angle)
 *   9. Sets +0x10EC = 1 (active state = true)
 *
 * Name verification: Vtable PTR_Rotator_DeletingDtor2 confirms "Rotator" is correct.
 * This is the vtable2 variant — there are 3 Rotator vtable variants (0x4D56A8, 0x4D5708, 0x4D5770).
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4/+0x10D8/+0x10DC: position (X/Y/Z)
 *   +0x10E0: rotation speed/extra param
 *   +0x10E4: current rotation angle (0)
 *   +0x10EC: active state (1)
 *   +0x10ED: sound started flag (0)
 *   +0x10F0: CollisionLevel pointer (owned)
 *
 * Cross-references:
 *   Called from NeonCollisionEvents (0x410C98) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
