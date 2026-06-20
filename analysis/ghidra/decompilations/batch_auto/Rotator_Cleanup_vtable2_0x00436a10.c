/*
 * Function: Rotator_Cleanup_vtable2
 * Address: 0x00436A10
 * Signature: void __fastcall Rotator_Cleanup_vtable2(undefined4 *param_1)
 * Parameters:
 *   param_1: Rotator* this — the Rotator (vtable3 variant) object to clean up
 *
 * Description:
 * Cleanup function for Rotator objects using vtable variant 3 (PTR_Rotator_DeletingDtor3).
 * Sets vtable to 0x4D5770, then calls Level_Cleanup to free CollisionLevel and
 * other resources. Called by the scalar deleting destructor before memory is freed.
 *
 * Struct offsets:
 *   +0x00: vtable (set to 0x4D5770)
 *
 * Cross-references:
 *   Called from Rotator_DeletingDtor3 (0x43D403) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
