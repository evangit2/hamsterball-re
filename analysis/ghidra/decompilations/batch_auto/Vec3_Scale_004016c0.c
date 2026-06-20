/*
 * Function: Vec3_Scale
 * Address: 0x004016C0
 * Signature: void __thiscall Vec3_Scale(void *this, float *param_1, float param_2)
 *
 * Description:
 * Scales a Vec3 (this) by a scalar factor (param_2) and writes the result to
 * an output buffer (param_1). Note: this reads from 'this' and writes to 'param_1',
 * so it does NOT modify in-place — it's a scale-and-store operation.
 *
 * The Vec3 layout uses offsets:
 *   +0x00: X component (float)
 *   +0x04: Y component (float)
 *   +0x08: Z component (float)
 *
 * Note: this = source vector, param_1 = destination, param_2 = scalar.
 * Result: dest = source * scalar
 *
 * Cross-references:
 *   - Called from SpatialTree_ForEach (0x463880) — spatial partitioning calculations
 *   - Called from Transform_ScalarDtor (0x466D50) — transform computation
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_Scale(void *this,float *param_1,float param_2)

{
  float fVar1;
  float fVar2;
  
  fVar1 = *(float *)((int)this + 8);
  fVar2 = *(float *)((int)this + 4);
  *param_1 = param_2 * *(float *)this;
  param_1[1] = param_2 * fVar2;
  param_1[2] = param_2 * fVar1;
  return;
}
