/*
 * Function: Vec3_DivideByScalar
 * Address: 0x00401890
 * Signature: Vec3_DivideByScalar(...)
 *
 * Patterns: none identified. Calls: Vec3_DivideByScalar. Offsets: 0, Lines: 12
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Vec3_DivideByScalar(void *this,float param_1)

{
  float fVar1;
  
  fVar1 = _DAT_004cf310 / param_1;
  *(float *)this = fVar1 * *(float *)this;
  *(float *)((int)this + 4) = fVar1 * *(float *)((int)this + 4);
  *(float *)((int)this + 8) = fVar1 * *(float *)((int)this + 8);
  return;
}
