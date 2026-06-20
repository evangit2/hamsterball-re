/*
 * Function: QuitToDesktopDialog_Render
 * Address: 0x00443FC0
 * Signature: void __thiscall QuitToDesktopDialog_Render(void *this, void *param_1)
 * Parameters:
 *   this: QuitToDesktopDialog* — the dialog to render
 *   param_1: void* — graphics context for rendering
 *
 * Description:
 * Renders the "QUIT TO DESKTOP?" confirmation dialog. Draws:
 *   1. Full-screen dark overlay (0,0,800,600) with alpha 0x3F400000 (0.75)
 *   2. Background rectangle via Graphics_DrawRectAndReset
 *   3. Two UI rectangles via UI_DrawRectAndReset (border/frame)
 *   4. Text "QUIT TO DESKTOP?" centered via UI_DrawTextShadow
 *   5. Yes/No button text
 *
 * Uses Matrix_Scale4x4 for alpha/opacity control. Font_MeasureText for centering.
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D62C8 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
