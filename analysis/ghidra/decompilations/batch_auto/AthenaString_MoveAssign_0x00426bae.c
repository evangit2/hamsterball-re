
void __fastcall AthenaString_MoveAssign(undefined4 param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  int unaff_EBX;
  int unaff_EBP;
  uint unaff_ESI;
  undefined4 *puVar5;
  
  uVar3 = *(uint *)(unaff_EBP + 0xc);
  iVar4 = 0;
  if (uVar3 != 0) {
    if (*(uint *)(unaff_EBX + 0x18) < 0x10) {
      puVar1 = (undefined4 *)(unaff_EBX + 4);
    }
    else {
      puVar1 = *(undefined4 **)(unaff_EBX + 4);
    }
    puVar5 = param_2;
    for (uVar2 = uVar3 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
      *puVar5 = *puVar1;
      puVar1 = puVar1 + 1;
      puVar5 = puVar5 + 1;
    }
    for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
      *(undefined1 *)puVar5 = *(undefined1 *)puVar1;
      puVar1 = (undefined4 *)((int)puVar1 + 1);
      puVar5 = (undefined4 *)((int)puVar5 + 1);
    }
    unaff_ESI = *(uint *)(unaff_EBP + -0x14);
    iVar4 = *(int *)(unaff_EBP + 0xc);
  }
  if (0xf < *(uint *)(unaff_EBX + 0x18)) {
    _free(*(void **)(unaff_EBX + 4));
    param_2 = *(undefined4 **)(unaff_EBP + 8);
    iVar4 = *(int *)(unaff_EBP + 0xc);
  }
  puVar1 = (undefined4 *)(unaff_EBX + 4);
  *(undefined1 *)puVar1 = 0;
  *puVar1 = param_2;
  *(uint *)(unaff_EBX + 0x18) = unaff_ESI;
  *(int *)(unaff_EBX + 0x14) = iVar4;
  if (0xf < unaff_ESI) {
    puVar1 = param_2;
  }
  *(undefined1 *)((int)puVar1 + iVar4) = 0;
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return;
}

