/*
 * Function: Rotator_RemoveAndFree
 * Address: 0x00436FC0
 * Signature: void __fastcall Rotator_RemoveAndFree(int param_1)
 * Parameters:
 *   param_1: Rotator* this — the rotator/obstacle object to remove and free
 *
 * Description:
 * Removes a rotator/obstacle from the scene and frees its collision level.
 * Only executes if the obstacle hasn't already been removed (checks +0x10E5 == 0).
 *
 * Steps when triggered:
 *   1. Adjusts position Y: param_1+0x10D8 -= _DAT_004CF484 (a float constant,
 *      likely a sink/fall offset so the obstacle drops below the floor)
 *   2. Sets +0x10E5 = 1 (removed flag = true, prevents re-entry)
 *   3. Sets +0x10E4 = 1 (triggered/destroyed flag)
 *   4. Calls Gfx_SetRenderState twice:
 *      a. First call via App→Scene+0x10D0→+0x8B0→+0x18 with CollisionLevel pointer (+0x10E0)
 *      b. Second call via Scene+0x10EC with CollisionLevel pointer
 *   5. If CollisionLevel pointer (+0x10E0) is non-null, calls its vtable[0] with arg=1
 *      (likely the CollisionLevel destructor/free method)
 *   6. Sets +0x10E0 = 0 (CollisionLevel pointer cleared)
 *
 * This function is used when a rotator/platform obstacle needs to be removed
 * from the level (e.g., after a ball passes through a trigger or the obstacle
 * is destroyed). The position adjustment makes it visually sink/fall away.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer (chain to App and GfxSystem)
 *   +0x10D8: position Y (adjusted by sink offset)
 *   +0x10E0: CollisionLevel pointer (freed and cleared)
 *   +0x10E4: triggered flag (set to 1)
 *   +0x10E5: removed flag (0→1, prevents re-entry)
 *   +0x10EC: Scene+0x10EC (GfxSystem/render state pointer)
 *
 * Cross-references:
 *   Called from Scene_SetupLevel10 (0x412097) — UNCONDITIONAL_CALL
 *   Called from 0x413067 — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
