/*
 * Function: Rotator_Cleanup_vtable5
 * Address: 0x004371A0
 * Signature: void __fastcall Rotator_Cleanup_vtable5(undefined4 *param_1)
 * Parameters:
 *   param_1: Rotator* this — the Rotator (vtable6 variant) object to clean up
 *
 * Description:
 * Cleanup function for Rotator objects using vtable variant 6 (PTR_Rotator_DeletingDtor6).
 * Sets vtable to 0x4D59B0, then calls Level_Cleanup to free CollisionLevel and resources.
 *
 * Struct offsets:
 *   +0x00: vtable (set to 0x4D59B0)
 *
 * Cross-references:
 *   Called from Rotator_DeletingDtor6 (0x43E0F3) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
