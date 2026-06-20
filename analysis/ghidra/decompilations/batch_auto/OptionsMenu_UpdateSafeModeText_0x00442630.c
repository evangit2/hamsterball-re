/*
 * Function: OptionsMenu_UpdateSafeModeText
 * Address: 0x00442630
 * Signature: void __fastcall OptionsMenu_UpdateSafeModeText(void *param_1)
 * Parameters:
 *   param_1: void* — pointer to the UI list/menu containing the "SM" item
 *
 * Description:
 * Updates the "SAFE MODE" text in the options menu. Formats string via
 * AthenaString_SprintfToBuffer, then sets UIList item "SM" text.
 *
 * Cross-references:
 *   Called from 0x4438B5 — UNCONDITIONAL_CALL (OptionsMenu_Update)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
