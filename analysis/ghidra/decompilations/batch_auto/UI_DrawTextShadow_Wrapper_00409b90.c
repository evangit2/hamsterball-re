/*
 * Function: UI_DrawTextShadow_Wrapper
 * Address: 0x00409B90
 * Signature: void __thiscall UI_DrawTextShadow_Wrapper(void *this, ...)
 *
 * Description:
 * Convenience wrapper around UI_DrawTextShadow that handles the Vec3 vtable
 * parameter setup automatically. Passes PTR_Vec3_dtor_004cf300 (the Vec3 vtable)
 * as both the shadow and main color vtable parameters, then calls UI_DrawTextShadow
 * with all the original parameters.
 *
 * After drawing, resets two matrices to identity (cleanup) and restores the
 * SEH frame.
 *
 * Cross-references (26 call sites):
 *   - RumbleBoard_Render — arena score display
 *   - HighScoreMenu_Render (2 calls) — high score table
 *   - Many HUD and menu rendering functions
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
UI_DrawTextShadow_Wrapper(void *this, byte *param_1, int param_2, int param_3,
                         int param_4, int param_5, undefined4 param_6, undefined4 param_7,
                         undefined4 param_8, undefined4 param_9, undefined4 param_10,
                         undefined4 param_11, undefined4 param_12, undefined4 param_13,
                         undefined4 param_14, undefined4 param_15)
{
  /* ... see Ghidra decompilation ... */
}
