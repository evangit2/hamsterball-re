
void __fastcall InputDevice_PollAndRelease(int param_1)

{
  int iVar1;
  int *piVar2;
  undefined4 *puVar3;
  int iVar4;
  
  iVar4 = *(int *)(param_1 + 0x434);
  if (iVar4 != 0) {
    iVar1 = (**(code **)(**(int **)(iVar4 + 8) + 0x24))
                      (*(int **)(iVar4 + 8),0x100,(undefined4 *)(iVar4 + 0xc));
    if (iVar1 < 0) {
      (**(code **)(**(int **)(iVar4 + 8) + 0x1c))(*(int **)(iVar4 + 8));
      puVar3 = (undefined4 *)(iVar4 + 0xc);
      for (iVar1 = 0x40; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = 0;
        puVar3 = puVar3 + 1;
      }
    }
  }
  piVar2 = (int *)(param_1 + 0x424);
  iVar4 = 4;
  do {
    if (*piVar2 != 0) {
      GamepadDevice_PollState(*piVar2);
    }
    piVar2 = piVar2 + 1;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  return;
}

