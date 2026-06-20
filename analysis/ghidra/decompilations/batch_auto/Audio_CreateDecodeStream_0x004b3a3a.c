
void __cdecl Audio_CreateDecodeStream(int param_1,char param_2)

{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  
  puVar2 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x1c);
  *(undefined4 **)(param_1 + 0x188) = puVar2;
  *puVar2 = &LAB_004b3994;
  puVar2[2] = 0;
  puVar2[3] = 0;
  if (*(char *)(param_1 + 0x4a) != '\0') {
    iVar5 = *(int *)(param_1 + 0x110);
    puVar2[4] = iVar5;
    if (param_2 == '\0') {
      uVar4 = (**(code **)(*(int *)(param_1 + 4) + 8))
                        (param_1,1,*(int *)(param_1 + 100) * *(int *)(param_1 + 0x5c),iVar5);
      puVar2[3] = uVar4;
    }
    else {
      iVar1 = *(int *)(param_1 + 4);
      iVar3 = Math_AlignUp(*(int *)(param_1 + 0x60),iVar5);
      uVar4 = (**(code **)(iVar1 + 0x10))
                        (param_1,1,0,*(int *)(param_1 + 100) * *(int *)(param_1 + 0x5c),iVar3,iVar5)
      ;
      puVar2[2] = uVar4;
    }
  }
  return;
}

