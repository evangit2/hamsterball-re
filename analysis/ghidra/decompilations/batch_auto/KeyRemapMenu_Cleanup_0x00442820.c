/*
 * Function: KeyRemapMenu_Cleanup
 * Address: 0x00442820
 * Signature: void __fastcall KeyRemapMenu_Cleanup(undefined4 *param_1)
 * Parameters:
 *   param_1: KeyRemapMenu* this — the key remap menu to clean up
 *
 * Description:
 * Cleanup function for the KeyRemapMenu. Sets the vtable pointer to
 * PTR_KeyRemapMenu_DeletingDtor (0x4D5F50), then calls UIList_Cleanup
 * to release the base UIList resources.
 *
 * Cross-references:
 *   Called from KeyRemapMenu_DeletingDtor (0x443413) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
