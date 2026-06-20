/*
 * Function: RaceResultsMenu_scalar_dtor
 * Address: 0x0044ca80
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: RaceResultsMenu* this
 *
 * Description:
 * Scalar dtor for RaceResultsMenu. Sets vtable to 0x4D6CF0. Frees 2 sub-objects at +0x137 and +0x138 (0x4DC/+0x4E0 offsets) if non-NULL. Calls RumbleBoard_CleanupTimer on +0x131 (0x4C4 offset). Calls SceneObject_BaseDtor on +0x24. Sets vtable to 0x4D6C00. 1 call.
 *
 * Struct offsets:
 *   +0x137/+0x138 (sub-objects→0x4DC/0x4E0), +0x131 (timer→0x4C4), +0x24 (SceneObject)
 *
 * Cross-references:
 *   1 call from RaceGoalReached_DeletingDtor (0x44D743)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
