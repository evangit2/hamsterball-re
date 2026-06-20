/*
 * Function: Gfx_SetPositionAndRender
 * Address: 0x00457b80
 * Signature: void ...(undefined4 param_1, undefined4 param_2, undefined4 param_3)
 * Parameters:
 *   param_1-3: x, y, z position
 *
 * Description:
 * Sets world position AND triggers render. Uses D3DX_ShaderDispatch_4 (translation+render). Calls Graphics_SetRenderState. 5+ refs.
 *
 * Struct offsets:
 *   N/A
 *
 * Cross-references:
 *   5+ refs from Level_RenderDynamicObjects, FollowBall_Update, vtables
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
