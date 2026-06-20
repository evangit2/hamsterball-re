/*
 * Function: Looper_ctor
 * Address: 0x00435800
 * Signature: void * __thiscall Looper_ctor(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, void *param_5)
 * Parameters:
 *   this: Looper* — the Looper obstacle being constructed
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: undefined4 — stored at this+0x10D8 (position X)
 *   param_3: undefined4 — stored at this+0x10DC (position Y)
 *   param_4: undefined4 — stored at this+0x10E0 (position Z)
 *   param_5: void* — passed to Stands_ctor (parent scene, used to access MeshWorld via +0x878→+0x5C8)
 *
 * Description:
 * Constructor for Looper obstacle objects. Loopers are mechanical loop/track
 * obstacles found in levels like Expert Race. Steps:
 *   1. Calls Stands_ctor(this, param_5) — inherits from Stands base
 *   2. Sets vtable to PTR_PopCylinder_DeletingDtor (0x4D54B8)
 *      NOTE: Despite the name "Looper_ctor", the vtable says "PopCylinder" —
 *      Looper may share the same vtable as PopCylinder, or this is a misnomer.
 *   3. Initializes AthenaList at +0x10E8 (sub-objects)
 *   4. Stores parent Scene at +0x10D0
 *   5. Stores position at +0x10D8/+0x10DC/+0x10E0
 *   6. Sets +0x10E4 = 0 (initial state = inactive)
 *   7. Creates CollisionLevel (0x10D0 bytes) via operator_new + CollisionLevel_ctorWithLevel
 *   8. Copies field +0x434 to CollisionLevel+0x434 (arena/difficulty)
 *   9. Clears CollisionLevel+0x431 (collision flag = 0)
 *
 * Name verification: Ghidra labels this "Looper_ctor" and it's called from
 * CreateMechanicalObjects when the object name matches "LOOPER" (6 chars).
 * However, the vtable assignment (PTR_PopCylinder_DeletingDtor) suggests Looper
 * and PopCylinder share the same destruction vtable, or Looper inherits from
 * PopCylinder. The separate PopCylinder_Level_Dtor at 0x4358E0 uses the same vtable.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4: CollisionLevel pointer (owned, 0x10D0 bytes)
 *   +0x10D8/+0x10DC/+0x10E0: position (X/Y/Z)
 *   +0x10E4: state (0 = inactive)
 *   +0x10E8: AthenaList (sub-objects)
 *
 * Cross-references:
 *   Called from CreateMechanicalObjects (0x41805F) — UNCONDITIONAL_CALL
 *   CreateMechanicalObjects matches "LOOPER" prefix to call this constructor
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
