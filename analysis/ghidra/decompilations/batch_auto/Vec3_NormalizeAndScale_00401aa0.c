/*
 * Function: Vec3_NormalizeAndScale
 * Address: 0x00401aa0
 * Signature: Vec3_NormalizeAndScale(...)
 *
 * Patterns: none identified. Calls: Vec3_NormalizeAndScale, SQRT. Offsets: 0, Lines: 19
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Vec3_NormalizeAndScale(void *this,float param_1)

{
  float fVar1;
  float fVar2;
  
  fVar1 = *(float *)((int)this + 8) * *(float *)((int)this + 8) +
          *(float *)((int)this + 4) * *(float *)((int)this + 4) + *(float *)this * *(float *)this;
  fVar2 = _DAT_004cf368;
  if ((fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) &&
     (fVar2 = SQRT(fVar1), _DAT_004cf368 < fVar2)) {
    fVar2 = param_1 / fVar2;
  }
  *(float *)this = fVar2 * *(float *)this;
  *(float *)((int)this + 4) = fVar2 * *(float *)((int)this + 4);
  *(float *)((int)this + 8) = fVar2 * *(float *)((int)this + 8);
  return;
}
