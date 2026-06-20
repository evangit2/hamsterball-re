/*
 * Function: UI_DrawRectAndReset
 * Address: 0x00409AF0
 * Signature: void __thiscall UI_DrawRectAndReset(void *this)
 *
 * Description:
 * Draws a screen-space rectangle and resets the transform matrix. The rectangle
 * coordinates are passed via the FPU stack (4 float values converted to int via
 * __ftol2): x, y, width, height.
 *
 * Logic:
 *   1. Converts 4 FPU values to integers via __ftol2 (float-to-int truncation)
 *   2. Calls Graphics_DrawScreenRect(this, x, y, w, h) to render the rectangle
 *   3. Resets a matrix to identity (cleanup)
 *
 * Uses SEH frame for exception safety.
 *
 * Cross-references (46 call sites — one of the most used UI functions):
 *   - UIList_Render (2 calls) — list box backgrounds
 *   - HighScoreEntry_Render — score display backgrounds
 *   - Many menu and HUD rendering functions
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall UI_DrawRectAndReset(void *this)
{
  /* ... see Ghidra decompilation ... */
}
