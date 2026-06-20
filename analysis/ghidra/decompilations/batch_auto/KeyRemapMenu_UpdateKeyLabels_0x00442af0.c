/*
 * Function: KeyRemapMenu_UpdateKeyLabels
 * Address: 0x00442AF0
 * Signature: void __fastcall KeyRemapMenu_UpdateKeyLabels(void *param_1)
 * Parameters:
 *   param_1: void* — pointer to the KeyRemapMenu UIList
 *
 * Description:
 * Updates all key binding display labels in the Key Remap menu. For each control:
 *   1. Formats display string via AthenaString_SprintfToBuffer
 *   2. Sets the UIList item text via UIList_SetTextByName
 *
 * Labels updated:
 *   "Key Up:           %s" → item "UP"
 *   "Key Down:  %s"         → item "DOWN"
 *   "Key Left:       %s"     → item "LEFT"
 *   "Key Right:   %s"       → item "RIGHT"
 *   "Key Action1:    %s"     → item "ACTION1"
 *   "Key Action2:   %s"      → item "ACTION2"
 *
 * The "%s" is replaced by the current key name (e.g., "SPACE", "ENTER").
 *
 * Cross-references:
 *   Called from KeyRemapMenu_Ctor (0x4433EA) — UNCONDITIONAL_CALL (initial labels)
 *   Called from KeyRemapMenu_WaitForKey (0x4434CB) — UNCONDITIONAL_CALL (after remap)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
