/*
 * Function: Ball_CheckProximity
 * Address: 0x00402150
 * Signature: Ball_CheckProximity(...)
 *
 * Patterns: ball. Calls: Ball_CheckProximity, SQRT, __ftol2. Offsets: 4, Lines: 19
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Ball_CheckProximity(void *this,float param_1,float param_2,float param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  ulonglong uVar4;
  
  fVar2 = param_1 - *(float *)((int)this + 0x164);
  fVar1 = param_2 - *(float *)((int)this + 0x168);
  fVar3 = param_3 - *(float *)((int)this + 0x16c);
  param_1 = (float)(uint)(SQRT(fVar2 * fVar2 + fVar1 * fVar1 + fVar3 * fVar3) < _DAT_004cf3d8);
  if ((float)(int)param_1 != _DAT_004cf368) {
    uVar4 = __ftol2();
    *(int *)((int)this + 0x744) = (int)uVar4;
  }
  return;
}
