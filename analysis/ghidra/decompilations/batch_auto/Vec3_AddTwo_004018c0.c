/*
 * Function: Vec3_AddTwo
 * Address: 0x004018C0
 * Signature: void __thiscall Vec3_AddTwo(void *this, float *param_1, float *param_2)
 *
 * Description:
 * Adds two Vec3 vectors: this + param_2, and stores the result in param_1.
 * This is a three-way operation: result = vec_a + vec_b, where:
 *   this    = input vector A (source)
 *   param_2 = input vector B (source)
 *   param_1 = output vector (destination)
 *
 * Layout:
 *   +0x00: X (float)
 *   +0x04: Y (float)
 *   +0x08: Z (float)
 *
 * Result: param_1 = this + param_2
 *
 * Cross-references:
 *   - Called from SpatialTree_ForEach (0x463880) — spatial queries adding vectors
 *   - Called from Transform_ScalarDtor (0x466D50) — transform accumulation
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_AddTwo(void *this,float *param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  
  fVar1 = param_2[2];
  fVar2 = *(float *)((int)this + 8);
  fVar3 = param_2[1];
  fVar4 = *(float *)((int)this + 4);
  *param_1 = *param_2 + *(float *)this;
  param_1[1] = fVar3 + fVar4;
  param_1[2] = fVar1 + fVar2;
  return;
}
