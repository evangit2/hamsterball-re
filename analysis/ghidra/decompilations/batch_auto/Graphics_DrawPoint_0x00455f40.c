/*
 * Function: Graphics_DrawPoint
 * Address: 0x00455f40
 * Signature: void __thiscall ...(void *this)
 * Parameters:
 *   this: Graphics* (params passed via stack/registers)
 *
 * Description:
 * Draws a single point. Sets D3DRS_FILLMODE(0x16). Calls D3D vtable+0xCC (DrawPrimitive). Random RGBA color. 4+ calls.
 *
 * Struct offsets:
 *   +0x154 (D3D device→vtable+0xCC, +200)
 *
 * Cross-references:
 *   4+ calls from UIList_Render, HighScoreEntry_Render
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
