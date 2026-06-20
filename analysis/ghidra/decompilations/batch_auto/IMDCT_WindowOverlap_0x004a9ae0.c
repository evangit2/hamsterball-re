
void __thiscall IMDCT_WindowOverlap(void *this,int param_1,int param_2,int param_3)

{
  longlong lVar1;
  int iVar2;
  int iVar3;
  int in_EAX;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  int *piVar8;
  int iVar9;
  
  iVar3 = param_3;
  uVar7 = in_EAX - param_2;
  iVar9 = param_1 - (int)this;
  iVar4 = (int)uVar7 / iVar9;
  iVar5 = iVar4 + -1;
  if (-1 < (int)uVar7) {
    iVar5 = iVar4 + 1;
  }
  uVar6 = iVar4 * iVar9 >> 0x1f;
  lVar1 = (longlong)*(int *)(param_3 + (int)this * 4) *
          (longlong)*(int *)(&DAT_004e5ab8 + param_2 * 4);
  piVar8 = (int *)(&DAT_004e5ab8 + param_2 * 4);
  *(uint *)(param_3 + (int)this * 4) = (uint)lVar1 >> 0xf | (int)((ulonglong)lVar1 >> 0x20) << 0x11;
  param_3 = 0;
  while (this = (void *)((int)this + 1), (int)this < param_1) {
    param_3 = param_3 + (((uVar7 ^ (int)uVar7 >> 0x1f) - ((int)uVar7 >> 0x1f)) -
                        ((iVar4 * iVar9 ^ uVar6) - uVar6));
    iVar2 = iVar4;
    if (iVar9 <= param_3) {
      param_3 = param_3 - iVar9;
      iVar2 = iVar5;
    }
    piVar8 = piVar8 + iVar2;
    lVar1 = (longlong)*piVar8 * (longlong)*(int *)(iVar3 + (int)this * 4);
    *(uint *)(iVar3 + (int)this * 4) = (uint)lVar1 >> 0xf | (int)((ulonglong)lVar1 >> 0x20) << 0x11;
  }
  return;
}

