
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall ScoreDisplay_SetTime(void *this,int param_1)

{
  float fVar1;
  bool bVar2;
  int iVar3;
  ulonglong uVar4;
  char *pcVar5;
  
  iVar3 = RNG_Rand(&PTR_OBJ_VTABLE,2,'\x01');
  fVar1 = (float)(iVar3 + param_1);
  bVar2 = false;
  iVar3 = RNG_Rand(&PTR_OBJ_VTABLE,5,'\0');
  if (iVar3 == 0) {
    fVar1 = fVar1 + (float)_DAT_004cf3e0;
  }
  else {
    if (iVar3 != 1) {
      if (iVar3 == 2) {
        fVar1 = fVar1 + _DAT_004cf310;
      }
      goto LAB_00434ce2;
    }
    fVar1 = fVar1 - (float)_DAT_004cf3e0;
  }
  bVar2 = true;
LAB_00434ce2:
  if (((fVar1 < _DAT_004cf368) || (_DAT_004cf9f8 < fVar1)) || (!bVar2)) {
    pcVar5 = &DAT_004d5324;
  }
  else {
    pcVar5 = "%1.1f";
  }
  AthenaString_SprintfToBuffer((char *)((int)this + 0x10e8),(byte *)pcVar5);
  uVar4 = Font_MeasureText((char *)((int)this + 0x10e8));
  *(int *)((int)this + 0x10f0) = (int)uVar4 / 2;
  return;
}

