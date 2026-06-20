/*
 * Function: OptionsMenu_UpdateColorQualityText
 * Address: 0x004425E0
 * Signature: void __fastcall OptionsMenu_UpdateColorQualityText(void *param_1)
 * Parameters:
 *   param_1: void* — pointer to the UI list/menu containing the "CM" item
 *
 * Description:
 * Updates the "COLOR QUALITY" text in the options menu. Formats string via
 * AthenaString_SprintfToBuffer, then sets UIList item "CM" text.
 *
 * Cross-references:
 *   Called from 0x44388B — UNCONDITIONAL_CALL (OptionsMenu_Update)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
