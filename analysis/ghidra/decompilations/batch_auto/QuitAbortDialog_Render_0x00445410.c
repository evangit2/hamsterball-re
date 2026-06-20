/*
 * Function: QuitAbortDialog_Render
 * Address: 0x00445410
 * Signature: void __thiscall QuitAbortDialog_Render(void *this, void *param_1)
 * Parameters:
 *   this: QuitAbortDialog* — the dialog to render
 *   param_1: void* — graphics context (also sets param_1+0x708 = 1)
 *
 * Description:
 * Renders the quit abort confirmation dialog. Draws dark overlay, background
 * rectangle, UI frame, and centered text. Sets param_1+0x708 = 1 (modal flag).
 * Similar to other quit dialog renders but uses vtable dispatch for initial
 * render call (param_1+0x154→vtable+200).
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D64D0 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
