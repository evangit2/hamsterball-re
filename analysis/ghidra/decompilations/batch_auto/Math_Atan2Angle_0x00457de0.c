
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __thiscall
Math_Atan2Angle(void *param_1,float param_2,float param_3,float param_4,float param_5)

{
  float fVar1;
  float fVar2;
  float10 fVar3;
  
  fVar3 = (float10)param_4 - (float10)param_2;
  fVar1 = (float)fVar3;
  fVar2 = param_5 - param_3;
  if (fVar2 == _DAT_004cf368) {
    if (fVar3 < (float10)_DAT_004cf368 != (fVar3 == (float10)_DAT_004cf368)) {
      return (float10)_DAT_004d8e64;
    }
    return (float10)_DAT_004d03a0;
  }
  fVar3 = (float10)fpatan(fVar3 / (float10)fVar2,(float10)1);
  fVar3 = ABS((fVar3 / (float10)*(float *)((int)param_1 + 4)) * (float10)_DAT_004d0418);
  if ((fVar1 < _DAT_004cf368) || (_DAT_004cf368 <= fVar2)) {
    if ((fVar1 < _DAT_004cf368) && (fVar2 < _DAT_004cf368)) {
      return (float10)_DAT_004cf44c - fVar3;
    }
    if ((_DAT_004cf368 <= fVar1) && (_DAT_004cf368 < fVar2)) {
      return (float10)_DAT_004d0418 - fVar3;
    }
    fVar3 = fVar3 + (float10)_DAT_004d0418;
  }
  return fVar3;
}

