
int __fastcall Mesh_AdjacencyIteratorNext(int *param_1)

{
  int iVar1;
  char cVar2;
  int iVar3;
  undefined3 extraout_var;
  int iVar4;
  uint uVar5;
  
  iVar1 = param_1[2];
  param_1[3] = param_1[4];
  iVar4 = *(int *)(*(int *)(param_1[5] + 0x44) + (iVar1 + param_1[4] + iVar1 * 2) * 4);
  param_1[2] = iVar4;
  iVar3 = iVar1;
  if (iVar4 != *param_1) {
    do {
      if (param_1[2] != -1) {
        iVar4 = FindInSmallIntArray(*(int *)(param_1[5] + 0x44) + param_1[2] * 0xc,iVar3);
        param_1[4] = iVar4;
        if ((char)param_1[6] == '\0') {
          uVar5 = iVar4 + 2;
        }
        else {
          uVar5 = iVar4 + 1;
        }
        param_1[4] = uVar5 % 3;
        return iVar1;
      }
      if ((char)param_1[6] == '\0') {
        return iVar1;
      }
      if (*(char *)((int)param_1 + 0x19) != '\0') {
        return iVar1;
      }
      *(undefined1 *)(param_1 + 6) = 0;
      param_1[2] = *param_1;
      cVar2 = FindInSmallIntArray32
                        ((int *)(*(int *)(param_1[5] + 0x40) + *param_1 * 0xc),param_1[1]);
      param_1[4] = (CONCAT31(extraout_var,cVar2) + 2U) % 3;
      param_1[3] = (param_1[3] + 2U) % 3;
      iVar3 = param_1[2];
      iVar4 = *(int *)(*(int *)(param_1[5] + 0x44) + (iVar3 * 3 + param_1[4]) * 4);
      param_1[2] = iVar4;
    } while (iVar4 != *param_1);
  }
  param_1[2] = -1;
  return iVar1;
}

