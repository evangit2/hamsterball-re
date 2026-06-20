/*
 * Function: Gfx_SetViewMatrix
 * Address: 0x00454a30
 * Signature: void __thiscall ...(void *this, float *param_1)
 * Parameters:
 *   this: Graphics* | param_1: float* (16-element view matrix)
 *
 * Description:
 * Copies 16 floats from param_1 to +0x264 (view matrix). If +0x7D2 (mirror flag): negates elements [0], [4], [8] (column 0). Calls D3D vtable+0x94 (SetTransform, type=2=view). Calls Matrix_ComputeFrustum on +0x748. 1 call.
 *
 * Struct offsets:
 *   +0x264 (view matrix, 16 floats), +0x7D2 (mirror flag), +0x748 (frustum), +0x154 (D3D device→vtable+0x94)
 *
 * Cross-references:
 *   1 call from Graphics_RenderScene (0x454C13)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
