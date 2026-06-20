/*
 * Function: Graphics_ClearViewport
 * Address: 0x00453900
 * Signature: void __thiscall ...(void *this, undefined4 param_1)
 * Parameters:
 *   this: Graphics* | param_1: undefined4 (clear color)
 *
 * Description:
 * Clears viewport via D3D device vtable+0x90 (Clear). If +0x182 flag set: clears with flags=7 (target+Z+stencil). Else: clears with flags=3, color=param_1, Z=1.0. 4+ calls.
 *
 * Struct offsets:
 *   +0x154 (D3D device ptr), +0x182 (clear mode flag)
 *
 * Cross-references:
 *   4+ calls from Graphics_InitRenderStates, LoadingScreenGadget_Render, TourneyResultsScreen_Render
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
