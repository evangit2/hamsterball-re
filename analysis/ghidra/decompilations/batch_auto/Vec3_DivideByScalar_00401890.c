/*
 * Function: Vec3_DivideByScalar
 * Address: 0x00401890
 * Signature: void __thiscall Vec3_DivideByScalar(void *this, float param_1)
 *
 * Description:
 * Divides all three components of a Vec3 (this) by a scalar (param_1), in-place.
 * Rather than dividing each component directly, it precomputes the reciprocal
 * (1.0 / param_1) using the constant _DAT_004cf310 (which is 1.0f), then
 * multiplies each component by the reciprocal. This is a common optimization
 * to avoid three division operations (which are slower than multiplications).
 *
 * Layout:
 *   +0x00: X (float)
 *   +0x04: Y (float)
 *   +0x08: Z (float)
 *
 * Cross-references:
 *   - Called from Transform_ScalarDtor (0x466D50) — during transform computation,
 *     likely normalizing or scaling vectors
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_DivideByScalar(void *this,float param_1)

{
  float fVar1;
  
  fVar1 = _DAT_004cf310 / param_1;
  *(float *)this = fVar1 * *(float *)this;
  *(float *)((int)this + 4) = fVar1 * *(float *)((int)this + 4);
  *(float *)((int)this + 8) = fVar1 * *(float *)((int)this + 8);
  return;
}
