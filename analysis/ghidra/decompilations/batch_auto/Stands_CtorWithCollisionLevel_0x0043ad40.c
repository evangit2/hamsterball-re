/*
 * Function: Stands_CtorWithCollisionLevel
 * Address: 0x0043AD40
 * Signature: void * __thiscall Stands_CtorWithCollisionLevel(void *this, int param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, char *param_5)
 * Parameters:
 *   this: Stands* — the stands object being constructed (vtable = Blockdawg)
 *   param_1: int — parent Scene pointer (accessed at +0x878→App for MeshWorld at +0x174)
 *   param_2: undefined4 — stored at this+0x10E0 (position X)
 *   param_3: undefined4 — stored at this+0x10E4 (position Y)
 *   param_4: undefined4 — stored at this+0x10E8 (position Z)
 *   param_5: char* — mesh name string passed to MeshWorld_ctor (e.g., "levels\\level-mesh")
 *
 * Description:
 * Constructor for Stands objects with a specific mesh and collision level.
 * Uses vtable PTR_Blockdawg_DeletingDtor (0x4D5458) — this is actually a
 * Blockdawg-type constructor despite the "Stands" prefix.
 *
 * Steps:
 *   1. Calls MeshWorld_ctor(this, App→+0x174, param_5) — loads the specified mesh
 *   2. Sets vtable to PTR_Blockdawg_DeletingDtor (0x4D5458)
 *   3. Constructs two 50-element arrays (0x418 bytes each) at +0x1104 and +0xDDB4
 *      using FUN_0040A870 as constructor and Vec3List_Free as destructor
 *   4. Initializes AthenaList at +0x1AA64
 *   5. Stores parent Scene at +0x10D0, position at +0x10E0/+0x10E4/+0x10E8
 *   6. Creates CollisionLevel (0x10D0 bytes) at +0x10D4
 *   7. Copies +0x434 to CollisionLevel+0x434, clears CollisionLevel+0x431
 *   8. Initializes a Timer, calls Gfx_SetPosition, SceneObject_CallUpdate/Render
 *   9. Sets +0x10D8 = 0 (state)
 *   10. Sets +0x10F8 = 0x14 (20 — timer/health value)
 *
 * Name verification: Vtable PTR_Blockdawg_DeletingDtor (0x4D5458) confirms this
 * constructs Blockdawg-type objects, not generic Stands. The two 50-element arrays
 * match the Blockdawg_Level_Dtor at 0x4353A0.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4: CollisionLevel pointer (owned)
 *   +0x10D8: state (0)
 *   +0x10E0/+0x10E4/+0x10E8: position (X/Y/Z)
 *   +0x10F8: timer/health (20)
 *   +0x1104: array of 50 × 0x418 bytes (collision data)
 *   +0xDDB4: array of 50 × 0x418 bytes (collision data)
 *   +0x1AA64: AthenaList (sub-objects)
 *
 * Cross-references:
 *   Called from 0x40F5E3 — UNCONDITIONAL_CALL (Scene_SetupLevel7/Neon Race)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
