/*
 * Function: PopCylinder_ctor
 * Address: 0x00436EE0
 * Signature: void * __thiscall PopCylinder_ctor(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, void *param_5)
 * Parameters:
 *   this: PopCylinder* — the PopCylinder obstacle being constructed
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: undefined4 — stored at this+0x10D4 (position X)
 *   param_3: undefined4 — stored at this+0x10D8 (position Y)
 *   param_4: undefined4 — stored at this+0x10DC (position Z)
 *   param_5: void* — passed to Stands_ctor (parent scene for MeshWorld access)
 *
 * Description:
 * Constructor for PopCylinder obstacle objects. Uses vtable
 * PTR_Rotator_DeletingDtor5 (0x4D58F0) — the 5th Rotator vtable variant.
 * Despite the name "PopCylinder_ctor", it uses a Rotator-family vtable.
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_5) — base class init
 *   2. Sets vtable to PTR_Rotator_DeletingDtor5 (0x4D58F0)
 *   3. Stores position at +0x10D4/+0x10D8/+0x10DC, parent Scene at +0x10D0
 *   4. Creates CollisionLevel (0x10D0 bytes) at +0x10E0
 *   5. Copies +0x434 to CollisionLevel+0x434, clears CollisionLevel+0x431
 *   6. Sets +0x10E4 = 1 (active flag)
 *   7. Sets +0x10E5 = 0 (triggered flag = false)
 *
 * Name verification: Vtable PTR_Rotator_DeletingDtor5 (5th Rotator variant).
 * PopCylinder shares the Rotator family vtable system.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4/+0x10D8/+0x10DC: position (X/Y/Z)
 *   +0x10E0: CollisionLevel pointer (owned)
 *   +0x10E4: active flag (1)
 *   +0x10E5: triggered flag (0)
 *
 * Cross-references:
 *   Called from CreateLevelObjects (0x412540) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
