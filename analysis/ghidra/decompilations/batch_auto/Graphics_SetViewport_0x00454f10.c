
void __thiscall Graphics_SetViewport(void *this,int param_1,int param_2)

{
  int iVar1;
  float *pfVar2;
  undefined4 *puVar3;
  float *pfVar4;
  undefined4 *puVar5;
  float local_ac;
  undefined1 auStack_a8 [80];
  float afStack_58 [9];
  void *pvStack_34;
  undefined4 uStack_2c;
  undefined4 uStack_20;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004ccbfb;
  pvStack_c = ExceptionList;
  iVar1 = *(int *)((int)this + 0x5c);
  ExceptionList = &pvStack_c;
  *(undefined4 *)((int)this + 0x798) = 0;
  *(undefined4 *)((int)this + 0x79c) = 0;
  *(float *)((int)this + 0x7a0) = (float)*(int *)(iVar1 + 0x15c);
  *(float *)((int)this + 0x7a4) = (float)*(int *)(iVar1 + 0x160);
  if (param_1 != 0) {
    *(float *)((int)this + 0x7a0) = (float)(*(int *)(iVar1 + 0x15c) / 2);
    if (param_1 < 0) {
      *(undefined4 *)((int)this + 0x798) = 0;
    }
    if (0 < param_1) {
      *(int *)((int)this + 0x798) = *(int *)(iVar1 + 0x15c) / 2;
    }
  }
  if (param_2 != 0) {
    *(float *)((int)this + 0x7a4) = (float)(*(int *)(iVar1 + 0x160) / 2);
    if (param_2 < 0) {
      *(undefined4 *)((int)this + 0x79c) = 0;
    }
    if (0 < param_2) {
      *(int *)((int)this + 0x79c) = *(int *)(iVar1 + 0x160) / 2;
    }
  }
  local_ac = *(float *)((int)this + 0x7a0) / *(float *)((int)this + 0x7a4);
  Matrix_BuildPerspectiveFOV
            (afStack_58 + 3,0.7853982,local_ac,*(float *)((int)this + 0x790),
             *(float *)((int)this + 0x794));
  (**(code **)(**(int **)((int)this + 0x154) + 0x94))(*(int **)((int)this + 0x154),3,afStack_58 + 3)
  ;
  pfVar2 = afStack_58;
  pfVar4 = (float *)((int)this + 0x2a4);
  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
    *pfVar4 = *pfVar2;
    pfVar2 = pfVar2 + 1;
    pfVar4 = pfVar4 + 1;
  }
  Matrix_ComputeFrustum(*(int *)((int)this + 0x748));
  (**(code **)(**(int **)((int)this + 0x154) + 0xa4))(*(int **)((int)this + 0x154),&stack0xffffff4c)
  ;
  __ftol2();
  __ftol2();
  (**(code **)(**(int **)((int)this + 0x154) + 0xa0))(*(int **)((int)this + 0x154),&stack0xffffff44)
  ;
  Timer_Init(&local_ac);
  uStack_20 = 0;
  (**(code **)(**(int **)((int)this + 0x154) + 0x94))(*(int **)((int)this + 0x154),0x100,auStack_a8)
  ;
  puVar3 = (undefined4 *)&stack0xffffff4c;
  puVar5 = (undefined4 *)((int)this + 0x224);
  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar5 = *puVar3;
    puVar3 = puVar3 + 1;
    puVar5 = puVar5 + 1;
  }
  Matrix_ComputeFrustum(*(int *)((int)this + 0x748));
  uStack_2c = 0xffffffff;
  Timer_Cleanup((undefined4 *)&stack0xffffff48);
  ExceptionList = pvStack_34;
  return;
}

