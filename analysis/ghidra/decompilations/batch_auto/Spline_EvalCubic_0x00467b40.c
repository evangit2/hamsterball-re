
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __thiscall Spline_EvalCubic(void *param_1,float param_2,int param_3)

{
  int iVar1;
  int iVar2;
  undefined2 unaff_SI;
  float10 fVar3;
  ulonglong uVar4;
  undefined4 local_4;
  
  FPU_RoundDouble((double)param_2,unaff_SI);
  uVar4 = __ftol2();
  local_4 = (int)uVar4;
  if (local_4 < 0) {
    fVar3 = (float10)_DAT_004cf368;
    local_4 = 0;
  }
  else {
    iVar1 = *(int *)((int)param_1 + 4);
    if (iVar1 == 0) {
      iVar2 = 0;
    }
    else {
      iVar2 = *(int *)((int)param_1 + 8) - iVar1 >> 2;
    }
    if (local_4 < iVar2 + -1) {
      fVar3 = (float10)param_2;
    }
    else {
      if (iVar1 == 0) {
        iVar1 = 0;
      }
      else {
        iVar1 = *(int *)((int)param_1 + 8) - iVar1 >> 2;
      }
      local_4 = iVar1 + -2;
      fVar3 = (float10)(iVar1 + -1);
    }
  }
  fVar3 = fVar3 - (float10)local_4;
  iVar1 = *(int *)(param_3 + 4);
  return (float10)*(float *)(iVar1 + 4 + local_4 * 0x10) * fVar3 * fVar3 +
         fVar3 * fVar3 * fVar3 * (float10)*(float *)(iVar1 + local_4 * 0x10) +
         (float10)*(float *)(iVar1 + 8 + local_4 * 0x10) * fVar3 +
         (float10)*(float *)(iVar1 + 0xc + local_4 * 0x10);
}

