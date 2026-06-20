/*
 * Function: QuitToDesktop_Render
 * Address: 0x00444880
 * Signature: void __thiscall QuitToDesktop_Render(void *this, void *param_1)
 * Parameters:
 *   this: QuitToDesktopDialog* — the dialog to render
 *   param_1: void* — graphics context
 *
 * Description:
 * Renders the "REALLY ABORT?" confirmation dialog. Similar layout to
 * QuitToDesktopDialog_Render but with "REALLY ABORT?" text. Draws:
 *   1. Full-screen dark overlay (0,0,800,600)
 *   2. Background rect via Graphics_DrawRectAndReset
 *   3. UI frame rectangles
 *   4. Text "REALLY ABORT?" centered via UI_DrawTextShadow
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D6370 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
