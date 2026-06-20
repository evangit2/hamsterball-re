/*
 * Function: Graphics_SetRenderMode
 * Address: 0x00454190
 * Signature: void __thiscall ...(void *this, char param_1)
 * Parameters:
 *   this: Graphics* | param_1: char (render mode: 0=software, 1=hardware)
 *
 * Description:
 * Sets D3D render mode via vtable+200 (state 7=D3DRS_FILLMODE). If mode changed: increments dirty counter +0x7C8. If +0x6FC flag set: resets vertex processing, sets D3DRS_STENCILENABLE(0x16=1), D3DRS_ZENABLE(0x1D=0), D3DRS_ALPHATESTENABLE(0x89=0). If +0x700==1: sets D3DRS_CLIPPING(0x1B=1). 4+ calls.
 *
 * Struct offsets:
 *   +0x154 (D3D device), +0x6FC (reset flag), +0x700 (clipping mode), +0x70D (current mode), +0x7C8 (dirty counter)
 *
 * Cross-references:
 *   4+ calls from Sprite_DrawRect, Gfx_DrawQuadRandomColor, Graphics_DrawScreenRect
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
