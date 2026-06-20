/*
 * Function: Gfx_SetPosition
 * Address: 0x00457b50
 * Signature: void ...(undefined4 param_1, undefined4 param_2, undefined4 param_3)
 * Parameters:
 *   param_1-3: x, y, z position
 *
 * Description:
 * Sets world position via D3DX_ShaderDispatch_4b (translation matrix). Calls Graphics_SetRenderState. 4+ calls.
 *
 * Struct offsets:
 *   N/A (uses stack-local matrix)
 *
 * Cross-references:
 *   4+ calls from Scene_RenderWithCamera
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
