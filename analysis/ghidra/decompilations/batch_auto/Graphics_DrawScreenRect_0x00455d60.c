/*
 * Function: Graphics_DrawScreenRect
 * Address: 0x00455d60
 * Signature: void __thiscall ...(void *this, int param_1, int param_2, int param_3, int param_4)
 * Parameters:
 *   this: Graphics* | param_1-4: int (x, y, w, h)
 *
 * Description:
 * Draws a colored rectangle on screen. Multiplies coords by parent+0x1F8/+0x1FC (scale). Random RGBA color via Color_RandomRGBA. 4+ calls.
 *
 * Struct offsets:
 *   +0x5C (parent→+0x1F8/+0x1FC: scale)
 *
 * Cross-references:
 *   4+ calls from UI_DrawRectAndReset, CreditsScreen_Render, UIList_Render
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
