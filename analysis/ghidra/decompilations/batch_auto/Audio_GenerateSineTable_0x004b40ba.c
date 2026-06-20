
void Audio_GenerateSineTable(void)

{
  int iVar1;
  int in_EAX;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = *(int *)(in_EAX + 0x1a0);
  uVar2 = (*(code *)**(undefined4 **)(in_EAX + 4))();
  *(undefined4 *)(iVar1 + 8) = uVar2;
  uVar2 = (*(code *)**(undefined4 **)(in_EAX + 4))();
  *(undefined4 *)(iVar1 + 0xc) = uVar2;
  uVar2 = (*(code *)**(undefined4 **)(in_EAX + 4))();
  *(undefined4 *)(iVar1 + 0x10) = uVar2;
  uVar2 = (*(code *)**(undefined4 **)(in_EAX + 4))();
  *(undefined4 *)(iVar1 + 0x14) = uVar2;
  iVar3 = 0;
  iVar5 = 0x5b6900;
  local_c = -0xe25100;
  local_8 = -0xb2f480;
  iVar4 = 0x2c8d00;
  do {
    *(int *)(iVar3 + *(int *)(iVar1 + 8)) = local_8 >> 0x10;
    *(int *)(iVar3 + *(int *)(iVar1 + 0xc)) = local_c >> 0x10;
    *(int *)(iVar3 + *(int *)(iVar1 + 0x10)) = iVar5;
    *(int *)(iVar3 + *(int *)(iVar1 + 0x14)) = iVar4;
    iVar4 = iVar4 + -0x581a;
    iVar5 = iVar5 + -0xb6d2;
    iVar3 = iVar3 + 4;
    local_c = local_c + 0x1c5a2;
    local_8 = local_8 + 0x166e9;
  } while (-0x2b34e7 < iVar4);
  return;
}

