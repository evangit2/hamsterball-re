/*
 * Function: OptionsMenu_UpdateFullscreenText
 * Address: 0x00442590
 * Signature: void __fastcall OptionsMenu_UpdateFullscreenText(void *param_1)
 * Parameters:
 *   param_1: void* — pointer to the UI list/menu containing the "FS" item
 *
 * Description:
 * Updates the "FULLSCREEN" text in the options menu. Formats string via
 * AthenaString_SprintfToBuffer, then sets UIList item "FS" text.
 *
 * Cross-references:
 *   Called from 0x4438DF — UNCONDITIONAL_CALL (OptionsMenu_Update)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
