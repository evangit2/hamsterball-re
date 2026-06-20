
undefined4 __cdecl Audio_RenderSubbands(int param_1,int param_2,int *param_3,undefined4 *param_4)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  void *this;
  void *pvVar5;
  int iVar6;
  int iVar7;
  void *pvVar8;
  
  iVar1 = *(int *)(param_2 + 0x308);
  iVar4 = *(int *)(*(int *)(*(int *)(*(int *)(param_1 + 0x48) + 4) + 0x1c) +
                  *(int *)(param_1 + 0x20) * 4) / 2;
  if (param_3 == (int *)0x0) {
    for (; iVar4 != 0; iVar4 = iVar4 + -1) {
      *param_4 = 0;
      param_4 = param_4 + 1;
    }
    return 0;
  }
  iVar6 = *(int *)(iVar1 + 0x340) * *param_3;
  pvVar8 = (void *)0x0;
  param_1 = 1;
  this = (void *)0x0;
  iVar7 = iVar6;
  if (1 < *(int *)(param_2 + 0x2fc)) {
    do {
      iVar2 = *(int *)(param_2 + param_1 * 4);
      uVar3 = param_3[iVar2] & 0x7fff;
      pvVar5 = this;
      iVar6 = iVar7;
      if (uVar3 == param_3[iVar2]) {
        pvVar5 = *(void **)(iVar1 + 0x344 + iVar2 * 4);
        iVar6 = *(int *)(iVar1 + 0x340) * uVar3;
        IMDCT_WindowOverlap(this,(int)pvVar5,iVar7,(int)param_4);
        pvVar8 = pvVar5;
      }
      param_1 = param_1 + 1;
      this = pvVar5;
      iVar7 = iVar6;
    } while (param_1 < *(int *)(param_2 + 0x2fc));
  }
  for (; (int)pvVar8 < iVar4; pvVar8 = (void *)((int)pvVar8 + 1)) {
    param_4[(int)pvVar8] = param_4[(int)pvVar8] * iVar6;
  }
  return 1;
}

