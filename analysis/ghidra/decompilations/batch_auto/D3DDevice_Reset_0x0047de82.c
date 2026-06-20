
undefined4 D3DDevice_Reset(int *param_1,int *param_2,undefined4 *param_3)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  bool bVar6;
  
  puVar1 = (undefined4 *)param_1[0x19];
  if (puVar1 == (undefined4 *)0x0) {
    *param_3 = 0;
    iVar3 = 4;
    bVar6 = true;
    piVar4 = param_2;
    piVar5 = &DAT_004ee644;
    do {
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      bVar6 = *piVar4 == *piVar5;
      piVar4 = piVar4 + 1;
      piVar5 = piVar5 + 1;
    } while (bVar6);
    if (!bVar6) {
      iVar3 = 4;
      bVar6 = true;
      piVar4 = param_2;
      piVar5 = &DAT_004db22c;
      do {
        if (iVar3 == 0) break;
        iVar3 = iVar3 + -1;
        bVar6 = *piVar4 == *piVar5;
        piVar4 = piVar4 + 1;
        piVar5 = piVar5 + 1;
      } while (bVar6);
      if (!bVar6) {
        iVar3 = 4;
        bVar6 = true;
        piVar4 = &DAT_004db21c;
        do {
          if (iVar3 == 0) break;
          iVar3 = iVar3 + -1;
          bVar6 = *param_2 == *piVar4;
          param_2 = param_2 + 1;
          piVar4 = piVar4 + 1;
        } while (bVar6);
        if (!bVar6) {
          return 0x80004002;
        }
      }
    }
    *param_3 = param_1;
    (**(code **)(*param_1 + 4))(param_1);
    uVar2 = 0;
  }
  else {
    uVar2 = (**(code **)*puVar1)(puVar1,param_2,param_3);
  }
  return uVar2;
}

