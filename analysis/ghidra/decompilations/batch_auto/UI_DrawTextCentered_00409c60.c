/*
 * Function: UI_DrawTextCentered
 * Address: 0x00409C60
 * Signature: void __thiscall UI_DrawTextCentered(void *this, byte *text, int x, int y, ...)
 *
 * Description:
 * Draws text horizontally centered at position (x, y) on screen.
 * Measures the text width using Font_MeasureText, then subtracts the full width
 * from the X position (NOT half the width — this centers text at the LEFT edge
 * of the text, meaning the entire text extends to the right from x-width).
 *
 * Note: This differs from UI_DrawTextCenteredAbsolute (0x4013A0) which divides
 * the measured width by 2. This version subtracts the full width, which means
 * it right-aligns the text at position x rather than centering it. This may be
 * a naming discrepancy or the function serves a different purpose than its name
 * suggests.
 *
 * Cross-references (38 call sites — heavily used):
 *   - RumbleBoard_Render — arena score display
 *   - HighScoreMenu_Render — high score table entries
 *   - RaceResults_Render — race results screen
 *   - Many HUD and menu functions
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
UI_DrawTextCentered(void *this, byte *param_1, int param_2, int param_3,
                   int param_4, int param_5, undefined4 param_6, undefined4 param_7,
                   undefined4 param_8, undefined4 param_9, undefined4 param_10,
                   undefined4 param_11, undefined4 param_12, undefined4 param_13,
                   undefined4 param_14, undefined4 param_15)
{
  /* ... see Ghidra decompilation ... */
}
