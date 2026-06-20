
undefined4 __cdecl Huffman_ReadVCBHeader(uint *param_1,uint *param_2)

{
  uint uVar1;
  void *pvVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint *puVar6;
  
  puVar6 = param_2;
  for (iVar5 = 9; iVar5 != 0; iVar5 = iVar5 + -1) {
    *puVar6 = 0;
    puVar6 = puVar6 + 1;
  }
  uVar1 = BitStream_ReadBits(param_1,0x18);
  if (uVar1 == 0x564342) {
    uVar1 = BitStream_ReadBits(param_1,0x10);
    *param_2 = uVar1;
    uVar1 = BitStream_ReadBits(param_1,0x18);
    param_2[1] = uVar1;
    if (uVar1 != 0xffffffff) {
      uVar1 = BitStream_ReadBits(param_1,1);
      if (uVar1 == 0) {
        pvVar2 = _malloc(param_2[1] << 2);
        param_2[2] = (uint)pvVar2;
        uVar1 = BitStream_ReadBits(param_1,1);
        iVar5 = 0;
        if (uVar1 == 0) {
          if (0 < (int)param_2[1]) {
            do {
              uVar1 = BitStream_ReadBits(param_1,5);
              if (uVar1 == 0xffffffff) goto LAB_00499361;
              *(uint *)(param_2[2] + iVar5 * 4) = uVar1 + 1;
              iVar5 = iVar5 + 1;
            } while (iVar5 < (int)param_2[1]);
          }
        }
        else if (0 < (int)param_2[1]) {
          do {
            uVar1 = BitStream_ReadBits(param_1,1);
            if (uVar1 == 0) {
              *(undefined4 *)(param_2[2] + iVar5 * 4) = 0;
            }
            else {
              uVar1 = BitStream_ReadBits(param_1,5);
              if (uVar1 == 0xffffffff) goto LAB_00499361;
              *(uint *)(param_2[2] + iVar5 * 4) = uVar1 + 1;
            }
            iVar5 = iVar5 + 1;
          } while (iVar5 < (int)param_2[1]);
        }
      }
      else {
        if (uVar1 != 1) {
          return 0xffffffff;
        }
        uVar1 = BitStream_ReadBits(param_1,5);
        pvVar2 = _malloc(param_2[1] << 2);
        param_2[2] = (uint)pvVar2;
        iVar5 = 0;
        if (0 < (int)param_2[1]) {
          do {
            uVar1 = uVar1 + 1;
            iVar3 = BitUtils_CountLeadingZeros(param_2[1] - iVar5);
            uVar4 = BitStream_ReadBits(param_1,iVar3);
            if (uVar4 == 0xffffffff) goto LAB_00499361;
            iVar3 = 0;
            if (0 < (int)uVar4) {
              do {
                if ((int)param_2[1] <= iVar5) break;
                iVar3 = iVar3 + 1;
                *(uint *)(param_2[2] + iVar5 * 4) = uVar1;
                iVar5 = iVar5 + 1;
              } while (iVar3 < (int)uVar4);
            }
          } while (iVar5 < (int)param_2[1]);
        }
      }
      uVar1 = BitStream_ReadBits(param_1,4);
      param_2[3] = uVar1;
      if (uVar1 == 0) {
        return 0;
      }
      if ((0 < (int)uVar1) && ((int)uVar1 < 3)) {
        uVar1 = BitStream_ReadBits(param_1,0x20);
        param_2[4] = uVar1;
        uVar1 = BitStream_ReadBits(param_1,0x20);
        param_2[5] = uVar1;
        uVar1 = BitStream_ReadBits(param_1,4);
        param_2[6] = uVar1 + 1;
        uVar1 = BitStream_ReadBits(param_1,1);
        param_2[7] = uVar1;
        iVar5 = 0;
        if (param_2[3] == 1) {
          iVar5 = Huffman_CalcOptimalBitLen((int *)param_2);
        }
        else if (param_2[3] == 2) {
          iVar5 = param_2[1] * *param_2;
        }
        pvVar2 = _malloc(iVar5 * 4);
        iVar3 = 0;
        param_2[8] = (uint)pvVar2;
        if (0 < iVar5) {
          do {
            uVar1 = BitStream_ReadBits(param_1,param_2[6]);
            *(uint *)(param_2[8] + iVar3 * 4) = uVar1;
            iVar3 = iVar3 + 1;
          } while (iVar3 < iVar5);
        }
        if (iVar5 == 0) {
          return 0;
        }
        if (*(int *)((iVar5 * 4 - 4U) + param_2[8]) != -1) {
          return 0;
        }
      }
    }
  }
LAB_00499361:
  Huffman_ResetTable(param_2);
  return 0xffffffff;
}

