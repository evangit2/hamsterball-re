/*
 * Function: Rotator_ctor_sound
 * Address: 0x00436920
 * Signature: void * __thiscall Rotator_ctor_sound(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, void *param_5, int param_6)
 * Parameters:
 *   this: Rotator* — the rotator object being constructed (vtable variant 3)
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: undefined4 — stored at this+0x10D4 (position X)
 *   param_3: undefined4 — stored at this+0x10D8 (position Y)
 *   param_4: undefined4 — stored at this+0x10DC (position Z)
 *   param_5: void* — passed to Stands_ctor (parent scene for MeshWorld access)
 *   param_6: int — rotation speed or timer value. Stored at +0x10EC. If 0, defaults to 0x96 (150 decimal).
 *
 * Description:
 * Constructor for Rotator objects with sound support (vtable variant 3).
 * Uses PTR_Rotator_DeletingDtor3 (0x4D5770) — a third vtable variant.
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_5) — base class init
 *   2. Sets vtable to PTR_Rotator_DeletingDtor3 (0x4D5770)
 *   3. Stores position at +0x10D4/+0x10D8/+0x10DC, parent Scene at +0x10D0
 *   4. Stores rotation speed at +0x10EC (defaults to 150 if 0)
 *   5. Creates CollisionLevel (0x10D0 bytes) at +0x10E0
 *   6. Copies +0x434 to CollisionLevel+0x434, clears CollisionLevel+0x431
 *   7. Sets +0x10F0 = 1 (active/visible flag)
 *   8. Sets +0x10E4 = 0 (current rotation angle)
 *   9. Sets +0x10E8 = 0x3C (60 decimal — sound timer or effect duration)
 *
 * Name verification: Vtable PTR_Rotator_DeletingDtor3 confirms this is a Rotator
 * variant. The "_sound" suffix indicates this variant includes sound channel management.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4/+0x10D8/+0x10DC: position (X/Y/Z)
 *   +0x10E0: CollisionLevel pointer (owned)
 *   +0x10E4: current rotation angle (0)
 *   +0x10E8: sound/effect timer (60)
 *   +0x10EC: rotation speed (param_6 or 150 default)
 *   +0x10F0: active flag (1)
 *
 * Cross-references:
 *   Called from CreateSpeedCylinder (0x41183F) — UNCONDITIONAL_CALL
 *   Called from CreateLifter (0x414AAB) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
