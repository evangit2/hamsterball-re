
void __thiscall Mesh_ConnectElements(void *this,int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  
  iVar3 = param_1;
  iVar2 = *(int *)((int)this + 0x20);
  iVar1 = *(int *)((int)this + 0x28) + param_1 * 0x24;
  iVar4 = param_2 * 0x424 + iVar2;
  iVar5 = param_3 * 0x424 + iVar2;
  param_1 = 0;
  iVar6 = 0;
  if (0 < *(int *)(param_2 * 0x424 + 0x20 + iVar2)) {
    piVar7 = (int *)(iVar4 + 0x24);
    do {
      iVar6 = param_1;
      if (*piVar7 == param_3) break;
      iVar6 = param_1 + 1;
      piVar7 = piVar7 + 2;
      param_1 = iVar6;
    } while (iVar6 < *(int *)(iVar4 + 0x20));
  }
  if (iVar6 < *(int *)(iVar4 + 0x20)) {
    iVar2 = *(int *)(iVar4 + 0x28 + iVar6 * 8);
    *(int *)(*(int *)((int)this + 0x30) + 4 + iVar2 * 8) = iVar3;
    iVar6 = *(int *)(*(int *)((int)this + 0x30) + iVar2 * 8);
    iVar2 = *(int *)((int)this + 0x28) + iVar6 * 0x24;
    *(int *)(iVar1 + 0x10 + *(int *)(iVar1 + 0xc) * 4) = iVar6;
    *(int *)(iVar1 + 0xc) = *(int *)(iVar1 + 0xc) + 1;
    *(int *)(iVar2 + 0x10 + *(int *)(iVar2 + 0xc) * 4) = iVar3;
    *(int *)(iVar2 + 0xc) = *(int *)(iVar2 + 0xc) + 1;
    return;
  }
  iVar1 = *(int *)((int)this + 0x2c);
  *(int *)((int)this + 0x2c) = iVar1 + 1;
  *(int *)(iVar4 + 0x24 + *(int *)(iVar4 + 0x20) * 8) = param_3;
  *(int *)(iVar4 + 0x28 + *(int *)(iVar4 + 0x20) * 8) = iVar1;
  *(int *)(iVar4 + 0x20) = *(int *)(iVar4 + 0x20) + 1;
  *(int *)(iVar5 + 0x24 + *(int *)(iVar5 + 0x20) * 8) = param_2;
  *(int *)(iVar5 + 0x28 + *(int *)(iVar5 + 0x20) * 8) = iVar1;
  *(int *)(iVar5 + 0x20) = *(int *)(iVar5 + 0x20) + 1;
  *(int *)(*(int *)((int)this + 0x30) + iVar1 * 8) = iVar3;
  return;
}

