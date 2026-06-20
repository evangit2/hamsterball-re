/*
 * Function: OptionsMenu_AdjustVolume
 * Address: 0x00442680
 * Signature: void __thiscall OptionsMenu_AdjustVolume(void *this, char *param_1, int param_2)
 * Parameters:
 *   this: OptionsMenu* — the options menu containing volume settings
 *   param_1: char* — control name string identifying which volume to adjust:
 *     "SV" = Sound Volume (stored at +0xD10, flag at +0xD1C)
 *     "MV" = Music Volume (stored at +0xD48, flag at +0xD54)
 *     "TQ" = ??? (stored at +0xD80, flag at +0xD8C) — possibly "Texture Quality"
 *     "MS" = Master/Mute Setting (stored at +0xDF0, flag at +0xDFC)
 *   param_2: int — adjustment direction (positive = increase, negative = decrease)
 *
 * Description:
 * Adjusts audio/quality volume sliders in the Options menu. For each control:
 *   1. Reads current value from struct
 *   2. Adds param_2 × _DAT_004CF308 (step size multiplier)
 *   3. Clamps to range [_DAT_004CF368, _DAT_004CF310] (min/max bounds)
 *   4. Sets corresponding "dirty" flag to 1 (indicating value changed)
 *
 * For "MS" (Master): if value > _DAT_004CF310, sets it to max and returns early.
 *
 * Struct offsets:
 *   +0xD10: Sound Volume (float)
 *   +0xD1C: Sound Volume dirty flag (byte)
 *   +0xD48: Music Volume (float)
 *   +0xD54: Music Volume dirty flag (byte)
 *   +0xD80: Texture Quality / Third Volume (float)
 *   +0xD8C: dirty flag (byte)
 *   +0xDF0: Master Setting (float)
 *   +0xDFC: Master dirty flag (byte)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5E7C [DATA] — called as callback from UIList
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
