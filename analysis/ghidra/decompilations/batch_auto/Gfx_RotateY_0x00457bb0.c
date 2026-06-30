/*
 * Function: Gfx_RotateY
 * Address: 0x00457bb0
 * Signature: void ...(float param_1, float param_2, float param_3)
 * Parameters:
 *   param_1-3: float (rotation center x, y, z)
 *
 * Description:
 * Rotates around Y axis. Builds LookAt matrix with up=(0,1,0), eye=(-x,-y,-z). Calls D3D_Thunk_5 and Graphics_SetRenderState. 6+ refs.
 *
 * Struct offsets:
 *   N/A (stack-local matrix)
 *
 * Cross-references:
 *   6+ refs from ArenaObject_Update, vtables, Ball_Update
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
