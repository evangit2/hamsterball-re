
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall TimerArray_RenderTimed(int param_1)

{
  int *piVar1;
  float fVar2;
  int iVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
  ulonglong uVar7;
  undefined **ppuStackY_d8;
  undefined4 auStackY_d4 [12];
  undefined4 uStackY_a4;
  int iStack_84;
  float fStack_80;
  float fStack_7c;
  float fStack_78;
  undefined4 uStack_74;
  void *pvStack_3c;
  float fStack_20;
  float fStack_1c;
  float fStack_18;
  float fStack_14;
  float fStack_10;
  void *pvStack_c;
  int *piStack_8;
  float fStack_4;
  
  fStack_4 = -NAN;
  piStack_8 = (int *)&LAB_004cdd48;
  pvStack_c = ExceptionList;
  iVar3 = *(int *)(param_1 + 4);
  uStack_74 = 7;
  ExceptionList = &pvStack_c;
  (**(code **)(**(int **)(iVar3 + 0x154) + 200))();
  iStack_84 = 0x18;
  (**(code **)(**(int **)(iVar3 + 0x154) + 200))();
  (**(code **)(**(int **)(iVar3 + 0x154) + 200))();
  *(int *)(iVar3 + 0x7c8) = *(int *)(iVar3 + 0x7c8) + 1;
  fStack_7c = fStack_1c;
  fStack_80 = fStack_20;
  fStack_78 = fStack_18;
  fStack_20 = fStack_14 - fStack_20;
  fStack_1c = fStack_10 - fStack_1c;
  fStack_18 = (float)pvStack_c - fStack_18;
  uVar7 = __ftol2();
  Vec3_NormalizeAndScale(&fStack_20,1.0);
  Vec3_NormalizeAndScale(&fStack_20,fStack_4);
  piVar1 = piStack_8;
  uVar4 = 1;
  fVar2 = (float)uVar7;
  if (0 < (int)(float)uVar7) {
    do {
      fStack_4 = fVar2;
      Timer_Init(&uStack_74);
      piStack_8 = (int *)&ppuStackY_d8;
      ppuStackY_d8 = &PTR_Timer_dtor_004cf338;
      puVar5 = (undefined4 *)(uVar4 * 0x44 + 0xc + param_1);
      puVar6 = auStackY_d4;
      for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
        *puVar6 = *puVar5;
        puVar5 = puVar5 + 1;
        puVar6 = puVar6 + 1;
      }
      Gfx_SetRenderStateThunk();
      uStackY_a4 = 0x475c2b;
      Gfx_SetPosition(fStack_80,fStack_7c,fStack_78);
      (**(code **)(*piVar1 + 0x1c))();
      fStack_80 = fStack_20 + fStack_80;
      uVar4 = (uint)((char)uVar4 == '\0');
      fStack_7c = fStack_1c + fStack_7c;
      fStack_78 = fStack_18 + fStack_78;
      Timer_Cleanup(&uStack_74);
      fStack_4 = (float)((int)fStack_4 + -1);
      param_1 = iStack_84;
      fVar2 = fStack_4;
    } while (fStack_4 != 0.0);
  }
  iVar3 = *(int *)(param_1 + 4);
  uStackY_a4 = 0x475ca3;
  (**(code **)(**(int **)(iVar3 + 0x154) + 200))();
  *(int *)(iVar3 + 0x7c8) = *(int *)(iVar3 + 0x7c8) + 1;
  ExceptionList = pvStack_3c;
  return;
}

