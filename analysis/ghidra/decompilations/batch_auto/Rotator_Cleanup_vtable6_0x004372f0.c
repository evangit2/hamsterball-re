/*
 * Function: Rotator_Cleanup_vtable6
 * Address: 0x004372F0
 * Signature: void __fastcall Rotator_Cleanup_vtable6(undefined4 *param_1)
 * Parameters:
 *   param_1: Rotator* this — the Rotator (vtable7 variant) object to clean up
 *
 * Description:
 * Cleanup function for Rotator objects using vtable variant 7 (PTR_Rotator_DeletingDtor7).
 * Sets vtable to 0x4D5A10, then calls Level_Cleanup to free CollisionLevel and resources.
 * This is the 7th Rotator vtable variant in the Rotator family.
 *
 * Struct offsets:
 *   +0x00: vtable (set to 0x4D5A10)
 *
 * Cross-references:
 *   Called from Rotator_DeletingDtor7 (0x43E243) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
