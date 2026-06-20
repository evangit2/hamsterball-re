/*
 * Function: Graphics_DrawRectAndReset
 * Address: 0x00440DD0
 * Signature: void __thiscall Graphics_DrawRectAndReset(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5, undefined4 param_6)
 * Parameters:
 *   this: Graphics* — the graphics context
 *   param_1-param_6: undefined4 — rectangle parameters (4 float coordinates converted to int via __ftol2)
 *     These are screen-space coordinates: likely x, y, width, height, color1, color2
 *
 * Description:
 * Draws a screen-space rectangle and resets the transform matrix. Steps:
 *   1. Sets up SEH frame
 *   2. Converts 4 float parameters to int via __ftol2 (float-to-long conversion)
 *   3. Calls Graphics_DrawScreenRect(this, converted_params) — draws the rectangle
 *   4. Calls Matrix_Identity — resets the world transform matrix to identity
 *   5. Cleans up SEH frame
 *
 * This is used extensively by UI dialogs and menus to draw background panels,
 * buttons, and borders, then reset the transform for subsequent rendering.
 *
 * Struct offsets:
 *   N/A — operates on Graphics* this
 *
 * Cross-references:
 *   Called from OkayDialog_Render (0x44121B) — UNCONDITIONAL_CALL
 *   Called from QuitToDesktopDialog_Render (0x44402F) — UNCONDITIONAL_CALL
 *   Called from QuitToDesktop_Render (0x4448EF) — UNCONDITIONAL_CALL
 *   Called from QuitAbortDialog_Render (0x44549C) — UNCONDITIONAL_CALL
 *   Called from TourneyContinueDialog_Render (0x445FBF) — UNCONDITIONAL_CALL
 *   And many more dialog/menu render functions
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
