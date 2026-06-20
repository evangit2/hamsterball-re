/*
 * Function: Graphics_SetCullMode2
 * Address: 0x00453970
 * Signature: void __thiscall ...(void *this, undefined1 param_1)
 * Parameters:
 *   this: Graphics* | param_1: cull mode value
 *
 * Description:
 * Sets D3D cull mode via device vtable+200 (SetRenderState), state 0x1C (D3DRS_CULLMODE). If unaff_BL flag set: stores at +0x734. 4+ calls.
 *
 * Struct offsets:
 *   +0x154 (D3D device→vtable+200, state 0x1C), +0x734 (cull mode cache)
 *
 * Cross-references:
 *   4+ calls from Ball_InitRenderState, Level_InitScene, Gfx_SetupAlphaRenderState
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
