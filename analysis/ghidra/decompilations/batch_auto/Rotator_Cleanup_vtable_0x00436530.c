/*
 * Function: Rotator_Cleanup_vtable
 * Address: 0x00436530
 * Signature: void __fastcall Rotator_Cleanup_vtable(undefined4 *param_1)
 * Parameters:
 *   param_1: Rotator* this — the Rotator object to clean up
 *
 * Description:
 * Cleanup function for Rotator objects (vtable variant 1, PTR_Rotator_DeletingDtor).
 * Sets vtable pointer to PTR_Rotator_DeletingDtor (0x4D56A8), then calls Level_Cleanup
 * which frees the CollisionLevel and other level-owned resources. This is called
 * by the scalar deleting destructor before memory is freed.
 *
 * Struct offsets:
 *   +0x00: vtable (set to 0x4D56A8)
 *
 * Cross-references:
 *   Called from Rotator_DeletingDtor (0x43D123) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
