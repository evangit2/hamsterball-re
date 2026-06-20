
void __cdecl Audio_CodecFreeChannel(int *param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  
  iVar1 = param_1[1];
  if ((param_2 < 0) || (1 < param_2)) {
    *(undefined4 *)(*param_1 + 0x14) = 0xc;
    *(int *)(*param_1 + 0x18) = param_2;
    (**(code **)*param_1)(param_1);
  }
  if (param_2 == 1) {
    for (iVar5 = *(int *)(iVar1 + 0x40); iVar5 != 0; iVar5 = *(int *)(iVar5 + 0x24)) {
      if (*(char *)(iVar5 + 0x22) != '\0') {
        *(undefined1 *)(iVar5 + 0x22) = 0;
        (**(code **)(iVar5 + 0x30))(param_1,iVar5 + 0x28);
      }
    }
    *(undefined4 *)(iVar1 + 0x40) = 0;
    for (iVar5 = *(int *)(iVar1 + 0x44); iVar5 != 0; iVar5 = *(int *)(iVar5 + 0x24)) {
      if (*(char *)(iVar5 + 0x22) != '\0') {
        *(undefined1 *)(iVar5 + 0x22) = 0;
        (**(code **)(iVar5 + 0x30))(param_1,iVar5 + 0x28);
      }
    }
    *(undefined4 *)(iVar1 + 0x44) = 0;
  }
  puVar2 = *(undefined4 **)(iVar1 + 0x38 + param_2 * 4);
  *(undefined4 *)(iVar1 + 0x38 + param_2 * 4) = 0;
  while (puVar2 != (undefined4 *)0x0) {
    iVar5 = puVar2[2];
    iVar3 = puVar2[1];
    puVar4 = (undefined4 *)*puVar2;
    CRT_FreeParam2(param_1,puVar2);
    *(int *)(iVar1 + 0x48) = *(int *)(iVar1 + 0x48) - (iVar5 + 0x10 + iVar3);
    puVar2 = puVar4;
  }
  puVar2 = (undefined4 *)(iVar1 + 0x30 + param_2 * 4);
  puVar4 = (undefined4 *)*puVar2;
  *puVar2 = 0;
  while (puVar4 != (undefined4 *)0x0) {
    iVar5 = puVar4[2];
    iVar3 = puVar4[1];
    puVar2 = (undefined4 *)*puVar4;
    CRT_FreeParam2(param_1,puVar4);
    *(int *)(iVar1 + 0x48) = *(int *)(iVar1 + 0x48) - (iVar5 + 0x10 + iVar3);
    puVar4 = puVar2;
  }
  return;
}

