/*
 * Function: Graphics_SetViewport
 * Address: 0x00454b50
 * Signature: void __thiscall ...(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3)
 * Parameters:
 *   this: Graphics* | param_1-3: viewport params (x, y, dimensions)
 *
 * Description:
 * Sets viewport: stores base ptr (this+0x224) at +0x264, param_2 at +0x268, param_3 at +0x26C. Calls D3D_Thunk_0. 2+ calls.
 *
 * Struct offsets:
 *   +0x224 (viewport base), +0x264 (ptr to viewport), +0x268 (param_2), +0x26C (param_3)
 *
 * Cross-references:
 *   2+ calls from 0x402AF6, Ball_Update
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
