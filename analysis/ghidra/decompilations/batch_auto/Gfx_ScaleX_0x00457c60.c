/*
 * Function: Gfx_ScaleX
 * Address: 0x00457c60
 * Signature: void ...(float param_1)
 * Parameters:
 *   param_1: float (X scale factor)
 *
 * Description:
 * Scales X axis by param_1 × _DAT_004D8E58. Calls D3DX_ShaderDispatch_2b then Graphics_SetRenderState. 4+ calls.
 *
 * Struct offsets:
 *   N/A
 *
 * Cross-references:
 *   4+ calls from Scene_RenderWithCamera, Level_RenderDynamicObjects, TowerLevel_Ctor
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
