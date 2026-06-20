/*
 * Function: Catapult_ctor
 * Address: 0x00437E10
 * Signature: void * __thiscall Catapult_ctor(void *this, undefined4 param_1, int param_2)
 * Parameters:
 *   this: Catapult* — the catapult/launch pad obstacle being constructed
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: int — passed to Stands_ctor (parent scene for MeshWorld access via +0x878→+0x5C8)
 *
 * Description:
 * Constructor for Catapult obstacle objects (launch pads that boost balls).
 * Uses vtable PTR_MeshNode_Level_DeleteDtor3 (0x4D4F98) — shares vtable with
 * MeshNode Level objects, suggesting Catapult inherits from MeshNode/Level.
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_2) — base class init
 *   2. Sets vtable to PTR_MeshNode_Level_DeleteDtor3 (0x4D4F98)
 *   3. Stores parent Scene at +0x10D0
 *   4. Initializes position to (0, 0, 0) at +0x10D8/+0x10DC/+0x10E0
 *   5. Sets +0x10E4 = 0 (rotation/state)
 *   6. Sets +0x10E8 = 0xBF800000 (float -1.0 — launch direction Y = upward)
 *   7. Creates CollisionLevel (0x10D0 bytes) at +0x10D4
 *   8. Copies +0x434 to CollisionLevel+0x434, clears CollisionLevel+0x431
 *   9. Sets +0x10F0 = 0 (triggered flag)
 *   10. Sets +0x10F8 = 0 (timer)
 *   11. Sets +0x1100 = 0 (active state)
 *   12. Sets +0x1104 = 0x41880000 (float 17.0 — launch power/force magnitude)
 *
 * Name verification: Vtable PTR_MeshNode_Level_DeleteDtor3 confirms MeshNode/Level
 * inheritance. "Catapult" is the game object type name from MESHWORLD level data.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4: CollisionLevel pointer (owned)
 *   +0x10D8/+0x10DC/+0x10E0: position (initialized to 0,0,0)
 *   +0x10E4: rotation/state (0)
 *   +0x10E8: launch direction Y (-1.0 = upward in Y-down system)
 *   +0x10F0: triggered flag (0)
 *   +0x10F8: timer (0)
 *   +0x1100: active state (0)
 *   +0x1104: launch power (17.0f)
 *
 * Cross-references:
 *   Called from CreateLevelObjects (0x412711) — UNCONDITIONAL_CALL
 *   Called from 0x40D81D — UNCONDITIONAL_CALL (Scene_SetupLevel5/Tower)
 *   Called from 0x4147C6 — UNCONDITIONAL_CALL (CreateWobbly1 or similar)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
