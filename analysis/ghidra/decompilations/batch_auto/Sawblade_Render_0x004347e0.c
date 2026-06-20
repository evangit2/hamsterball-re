
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Sawblade_Render(int param_1)

{
  int iVar1;
  void *pvVar2;
  int *piVar3;
  float fVar4;
  float fVar5;
  undefined1 auVar6 [10];
  undefined4 uVar7;
  double dVar8;
  int aiStack_58 [2];
  undefined4 local_50 [4];
  void *pvStack_40;
  undefined4 uStack_38;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbbe8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Timer_Init(local_50);
  local_4 = 0;
  Gfx_ScaleY(*(float *)(param_1 + 0x10f0));
  Gfx_ScaleX(*(float *)(param_1 + 0x10ec));
  Gfx_SetPosition(*(undefined4 *)(param_1 + 0x10d4),
                  *(float *)(param_1 + 0x10d8) - *(float *)(param_1 + 0x1110),
                  *(undefined4 *)(param_1 + 0x10dc));
  (**(code **)(**(int **)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x5b0) + 0x1c))();
  Matrix44_Zero((int)aiStack_58);
  (**(code **)(aiStack_58[0] + 0x18))();
  Gfx_ScaleX(*(float *)(param_1 + 0x10ec));
  uVar7 = *(undefined4 *)(param_1 + 0x10dc);
  dVar8 = (double)((*(float *)(param_1 + 0x10d8) - *(float *)(param_1 + 0x1110)) - _DAT_004cf528);
  auVar6 = Wave_Sin(&PTR_PTR_004f7188,*(float *)(param_1 + 0x10f0) + *(float *)(param_1 + 0x10f0));
  Gfx_SetPosition(*(undefined4 *)(param_1 + 0x10d4),
                  (float)((float10)dVar8 - (float10)auVar6 * (float10)_DAT_004cf378),uVar7);
  iVar1 = *(int *)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x174);
  (**(code **)(**(int **)(iVar1 + 0x154) + 200))();
  *(undefined4 *)(iVar1 + 0x708) = 1;
  pvVar2 = *(void **)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x174);
  fVar4 = *(float *)((int)pvVar2 + 0x790) + (float)_DAT_004cf308;
  *(float *)((int)pvVar2 + 0x790) = fVar4;
  fVar5 = *(float *)((int)pvVar2 + 0x794) + (float)_DAT_004cf308;
  *(float *)((int)pvVar2 + 0x794) = fVar5;
  Graphics_SetProjection(pvVar2,fVar4,fVar5);
  (**(code **)(**(int **)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x5b0 +
                         *(int *)(param_1 + 0x10f8) * 4) + 0x1c))(&stack0xffffff90,0);
  pvVar2 = *(void **)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x174);
  fVar4 = *(float *)((int)pvVar2 + 0x790) - (float)_DAT_004cf308;
  *(float *)((int)pvVar2 + 0x790) = fVar4;
  fVar5 = *(float *)((int)pvVar2 + 0x794) - (float)_DAT_004cf308;
  *(float *)((int)pvVar2 + 0x794) = fVar5;
  Graphics_SetProjection(pvVar2,fVar4,fVar5);
  iVar1 = *(int *)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x174);
  piVar3 = *(int **)(iVar1 + 0x154);
  if (*(char *)(iVar1 + 0x7d2) == '\0') {
    (**(code **)(*piVar3 + 200))(piVar3,0x16,3);
  }
  else {
    (**(code **)(*piVar3 + 200))(piVar3,0x16,2);
  }
  *(undefined4 *)(iVar1 + 0x708) = 3;
  uStack_38 = 0xffffffff;
  Timer_Cleanup((undefined4 *)&stack0xffffff7c);
  ExceptionList = pvStack_40;
  return;
}

