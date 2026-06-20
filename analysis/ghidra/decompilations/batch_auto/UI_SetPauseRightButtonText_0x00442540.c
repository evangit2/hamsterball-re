/*
 * Function: UI_SetPauseRightButtonText
 * Address: 0x00442540
 * Signature: void __fastcall UI_SetPauseRightButtonText(void *param_1)
 * Parameters:
 *   param_1: void* — pointer to the UI list/menu containing the "PWRB" item
 *
 * Description:
 * Updates the "PAUSE W/RIGHT BUTTON" text in the options menu. Formats
 * the string using AthenaString_SprintfToBuffer, then sets the text of the
 * UIList item identified by name "PWRB" via UIList_SetTextByName.
 *
 * The "%s" is replaced with the current setting value (ON/OFF) by the
 * AthenaString formatting system, which reads from the game's config state.
 *
 * Cross-references:
 *   Called from 0x443909 — UNCONDITIONAL_CALL (OptionsMenu_Update)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
