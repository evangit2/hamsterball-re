/*
 * Function: RepeatCall
 * Address: 0x00409D60
 * Signature: void RepeatCall(undefined4 param_1, undefined4 param_2, int param_3, undefined *param_4)
 *
 * Description:
 * Calls a function pointer (param_4) repeatedly, param_3 times.
 * This is a simple loop utility used to invoke a callback N times.
 *
 * The first two parameters (param_1, param_2) are not used directly by
 * RepeatCall itself — they are likely passed as context that the called
 * function can access via register state or stack.
 *
 * Cross-references (34 call sites — widely used):
 *   - D3DX_SkinMesh_ProcessWeights (2 calls) — skinning mesh vertices
 *   - DDSurface_Blt3PointFilter — surface blitting (3-point filter)
 *   - Various graphics processing functions
 *
 * This function is part of the D3DX/Direct3D utility layer, not the
 * Hamsterball game logic itself.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void RepeatCall(undefined4 param_1,undefined4 param_2,int param_3,undefined *param_4)

{
  if (-1 < param_3 + -1) {
    do {
      (*(code *)param_4)();
      param_3 = param_3 + -1;
    } while (param_3 != 0);
  }
  return;
}
