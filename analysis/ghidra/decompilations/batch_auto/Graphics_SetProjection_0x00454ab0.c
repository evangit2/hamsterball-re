/*
 * Function: Graphics_SetProjection
 * Address: 0x00454ab0
 * Signature: void __thiscall ...(void *this, float param_1, float param_2)
 * Parameters:
 *   this: Graphics* | param_1: float (near Z) | param_2: float (far Z)
 *
 * Description:
 * Sets projection matrix. Stores nearZ at +0x790, farZ at +0x794. Builds perspective FOV matrix (PI/4, aspectRatio=+0x7A0/+0x7A4, near, far). Calls D3D vtable+0x94 (SetTransform, type=3=projection). Copies 16 floats to +0x2A4. Calls Matrix_ComputeFrustum. 4+ calls.
 *
 * Struct offsets:
 *   +0x790 (nearZ), +0x794 (farZ), +0x7A0/+0x7A4 (aspect ratio), +0x2A4 (projection matrix), +0x748 (frustum), +0x154 (D3D device)
 *
 * Cross-references:
 *   4+ calls from Graphics_Initialize, Level_InitScene, Scene_RenderReflectiveObjects
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
