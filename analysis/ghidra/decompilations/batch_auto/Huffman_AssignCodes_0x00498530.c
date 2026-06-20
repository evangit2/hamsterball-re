
uint * __cdecl Huffman_AssignCodes(int param_1,int param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  uint *_Memory;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint *puVar6;
  uint local_84 [33];
  
  iVar5 = param_3;
  if (param_3 == 0) {
    iVar5 = param_2;
  }
  _Memory = _malloc(iVar5 << 2);
  puVar6 = local_84;
  for (iVar5 = 0x21; iVar5 != 0; iVar5 = iVar5 + -1) {
    *puVar6 = 0;
    puVar6 = puVar6 + 1;
  }
  iVar5 = 0;
  puVar6 = _Memory;
  if (0 < param_2) {
    do {
      iVar1 = *(int *)(param_1 + iVar5 * 4);
      if (iVar1 < 1) {
        if (param_3 == 0) {
          puVar6 = puVar6 + 1;
        }
      }
      else {
        uVar4 = local_84[iVar1];
        if ((iVar1 < 0x20) && (uVar4 >> ((byte)iVar1 & 0x1f) != 0)) {
          _free(_Memory);
          return (uint *)0x0;
        }
        *puVar6 = uVar4;
        puVar6 = puVar6 + 1;
        iVar3 = iVar1;
        do {
          if ((local_84[iVar3] & 1) != 0) {
            if (iVar3 == 1) {
              local_84[1] = local_84[1] + 1;
            }
            else {
              local_84[iVar3] = local_84[iVar3 + -1] << 1;
            }
            break;
          }
          local_84[iVar3] = local_84[iVar3] + 1;
          iVar3 = iVar3 + -1;
        } while (0 < iVar3);
        while ((iVar3 = iVar1 + 1, iVar3 < 0x21 && (uVar2 = local_84[iVar3], uVar2 >> 1 == uVar4)))
        {
          local_84[iVar3] = local_84[iVar1] << 1;
          uVar4 = uVar2;
          iVar1 = iVar3;
        }
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < param_2);
  }
  iVar5 = 0;
  puVar6 = _Memory;
  if (0 < param_2) {
    do {
      iVar1 = *(int *)(param_1 + iVar5 * 4);
      uVar4 = 0;
      iVar3 = 0;
      if (0 < iVar1) {
        do {
          uVar4 = uVar4 * 2 | *puVar6 >> ((byte)iVar3 & 0x1f) & 1;
          iVar3 = iVar3 + 1;
        } while (iVar3 < iVar1);
      }
      if ((param_3 == 0) || (iVar1 != 0)) {
        *puVar6 = uVar4;
        puVar6 = puVar6 + 1;
      }
      iVar5 = iVar5 + 1;
    } while (iVar5 < param_2);
  }
  return _Memory;
}

