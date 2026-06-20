/*
 * Function: Rotator_Cleanup_vtable4
 * Address: 0x00436FB0
 * Signature: void __fastcall Rotator_Cleanup_vtable4(undefined4 *param_1)
 * Parameters:
 *   param_1: Rotator* this — the Rotator (vtable5 variant) object to clean up
 *
 * Description:
 * Cleanup function for Rotator objects using vtable variant 5 (PTR_Rotator_DeletingDtor5).
 * Sets vtable to 0x4D58F0, then calls Level_Cleanup to free CollisionLevel and resources.
 * Called by the scalar deleting destructor.
 *
 * Struct offsets:
 *   +0x00: vtable (set to 0x4D58F0)
 *
 * Cross-references:
 *   Called from Rotator_DeletingDtor5 (0x43DEB3) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
