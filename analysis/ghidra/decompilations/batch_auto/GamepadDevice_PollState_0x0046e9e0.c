
void __fastcall GamepadDevice_PollState(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 unaff_EDI;
  undefined4 *puVar4;
  undefined1 *puVar5;
  undefined4 uStack_108;
  undefined4 uStack_104;
  undefined4 uStack_100;
  undefined4 uStack_fc;
  undefined4 uStack_f8;
  undefined4 uStack_f4;
  undefined4 auStack_f0 [60];
  
  piVar1 = *(int **)(param_1 + 8);
  iVar2 = (**(code **)(*piVar1 + 100))();
  if (iVar2 < 0) {
    iVar2 = (**(code **)(**(int **)(param_1 + 8) + 0x1c))(*(int **)(param_1 + 8));
    while (iVar2 == -0x7ff8ffe2) {
      iVar2 = (**(code **)(**(int **)(param_1 + 8) + 0x1c))(*(int **)(param_1 + 8));
    }
  }
  puVar5 = &stack0xfffffeec;
  iVar2 = (**(code **)(**(int **)(param_1 + 8) + 0x24))(*(int **)(param_1 + 8),0x110);
  if (iVar2 < 0) {
    *(undefined4 *)(param_1 + 0x10c) = 0;
    *(undefined4 *)(param_1 + 0x110) = 0;
    *(undefined4 *)(param_1 + 0x114) = 0;
    *(undefined4 *)(param_1 + 0x118) = 0;
    *(undefined4 *)(param_1 + 0x11c) = 0;
    *(undefined4 *)(param_1 + 0x120) = 0;
    *(undefined4 *)(param_1 + 0x124) = 0;
    *(undefined4 *)(param_1 + 0x128) = 0;
    *(undefined4 *)(param_1 + 300) = 0;
    puVar3 = (undefined4 *)(param_1 + 0x130);
    for (iVar2 = 0x20; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar3 = 0;
      puVar3 = puVar3 + 1;
    }
    return;
  }
  *(undefined4 *)(param_1 + 0x114) = unaff_EDI;
  *(undefined1 **)(param_1 + 0x10c) = puVar5;
  *(int **)(param_1 + 0x110) = piVar1;
  *(undefined4 *)(param_1 + 0x120) = uStack_100;
  *(undefined4 *)(param_1 + 0x118) = uStack_108;
  *(undefined4 *)(param_1 + 0x11c) = uStack_104;
  *(undefined4 *)(param_1 + 300) = uStack_f4;
  *(undefined4 *)(param_1 + 0x124) = uStack_fc;
  *(undefined4 *)(param_1 + 0x128) = uStack_f8;
  puVar3 = auStack_f0;
  puVar4 = (undefined4 *)(param_1 + 0x130);
  for (iVar2 = 0x20; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar4 = *puVar3;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  }
  return;
}

