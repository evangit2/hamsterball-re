
undefined4 __cdecl AthenaList_SortMerge(int *param_1,int *param_2)

{
  int iVar1;
  undefined1 uVar2;
  int iVar3;
  int3 extraout_var;
  
  iVar3 = AthenaList_Sort_14();
  uVar2 = StreamReader_ReadByte(param_2);
  if (iVar3 != param_1[6]) {
    if (param_2 != (int *)0x0) {
      Pool_FreeList((int *)*param_2);
      Pool_FreeList((int *)param_2[2]);
      *param_2 = 0;
      param_2[1] = 0;
      param_2[2] = 0;
      param_2[3] = 0;
    }
    return 0xfffffff3;
  }
  if (CONCAT31(extraout_var,uVar2) == 0 || extraout_var < 0) {
    if (param_1[3] == 0) {
      param_1[3] = param_2[2];
      iVar3 = param_2[2];
      if (iVar3 != 0) {
        for (iVar1 = *(int *)(iVar3 + 0xc); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0xc)) {
          iVar3 = iVar1;
        }
      }
    }
    else {
      iVar3 = List_AppendChain(param_1[2]);
    }
    param_1[2] = iVar3;
    if (param_1[1] == 0) {
      param_1[1] = *param_2;
      iVar3 = *param_2;
      if (iVar3 == 0) {
        param_1[0xc] = -0x1b;
      }
      else {
        for (iVar1 = *(int *)(iVar3 + 0xc); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0xc)) {
          iVar3 = iVar1;
        }
        param_1[0xc] = -0x1b;
      }
    }
    else {
      iVar3 = List_AppendChain(*param_1);
    }
    *param_1 = iVar3;
    *param_2 = 0;
    param_2[1] = 0;
    param_2[2] = 0;
    param_2[3] = 0;
    return 0;
  }
  if (param_2 != (int *)0x0) {
    Pool_FreeList((int *)*param_2);
    Pool_FreeList((int *)param_2[2]);
    *param_2 = 0;
    param_2[1] = 0;
    param_2[2] = 0;
    param_2[3] = 0;
  }
  return 0xfffffff4;
}

