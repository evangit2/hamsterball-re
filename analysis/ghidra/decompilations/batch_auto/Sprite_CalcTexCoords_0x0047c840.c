
void __fastcall Sprite_CalcTexCoords(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  ulonglong uVar4;
  int iStack00000004;
  
  iVar1 = *(int *)(param_1 + 0xc);
  iVar2 = *(int *)(param_1 + 0x14);
  iStack00000004 = iVar1 + iVar2;
  uVar4 = __ftol2();
  iVar3 = *(int *)(param_1 + 0x18);
  *(int *)(param_1 + 4) = (int)uVar4;
  iStack00000004 = *(int *)(param_1 + 0x10) + iVar3;
  uVar4 = __ftol2();
  *(int *)(param_1 + 8) = (int)uVar4;
  *(int *)(param_1 + 0x14) = iVar2 + 1;
  *(int *)(param_1 + 0xc) = iVar1 + 0x4d2;
  *(int *)(param_1 + 0x18) = iVar3 + 1;
  *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 0x4d2;
  return;
}

