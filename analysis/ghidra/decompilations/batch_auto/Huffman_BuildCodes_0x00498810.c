
int * __cdecl Huffman_BuildCodes(int *param_1,int param_2,int param_3,int *param_4)

{
  longlong lVar1;
  int *piVar2;
  uint uVar3;
  int *piVar4;
  void *_Memory;
  uint uVar5;
  int *piVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int local_40;
  int local_3c;
  int local_30;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_14;
  
  piVar2 = param_1;
  local_40 = 0;
  if ((param_1[3] != 1) && (param_1[3] != 2)) {
    return (int *)0x0;
  }
  Float16_ToFloat32(param_1[4]);
  uVar3 = Float16_ToFloat32(param_1[5]);
  piVar4 = _calloc(*param_1 * param_2,4);
  _Memory = _calloc(*param_1 * param_2,4);
  *param_4 = local_28;
  if (param_1[3] == 1) {
    iVar11 = Huffman_CalcOptimalBitLen(param_1);
    local_3c = 0;
    if (0 < param_1[1]) {
      do {
        if ((param_3 == 0) || (*(int *)(piVar2[2] + local_3c * 4) != 0)) {
          local_14 = 0;
          param_1 = (int *)0x0;
          iVar7 = *piVar2;
          local_24 = 1;
          if (0 < iVar7) {
            do {
              iVar9 = 0;
              uVar5 = *(uint *)(piVar2[8] + ((local_3c / local_24) % iVar11) * 4);
              uVar8 = (int)uVar5 >> 0x1f;
              uVar8 = (uVar5 ^ uVar8) - uVar8;
              for (uVar5 = (uVar8 ^ (int)uVar8 >> 0x1f) - ((int)uVar8 >> 0x1f); uVar5 != 0;
                  uVar5 = uVar5 >> 1) {
                iVar9 = iVar9 + 1;
              }
              iVar10 = uVar8 << (-((char)iVar9 + -0x1f) & 0x1fU);
              if ((uVar3 == 0) || (iVar10 == 0)) {
                lVar1 = 0;
              }
              else {
                local_30 = iVar9 + 1 + local_1c;
                lVar1 = (longlong)iVar10 * (longlong)(int)uVar3;
              }
              uVar5 = Huffman_BuildCombinedNode(local_30,(uint)((ulonglong)lVar1 >> 0x20),local_28);
              uVar5 = Huffman_BuildCombinedNode(local_30,uVar5,local_14);
              if (piVar2[7] != 0) {
                local_14 = local_30;
              }
              if (param_3 == 0) {
                piVar4[(int)(iVar7 * local_40 + (int)param_1)] = uVar5;
                *(int *)((int)_Memory + (int)(*piVar2 * local_40 + (int)param_1) * 4) = local_30;
              }
              else {
                piVar4[(int)(*(int *)(param_3 + local_40 * 4) * iVar7 + (int)param_1)] = uVar5;
                *(int *)((int)_Memory +
                        (int)(*piVar2 * *(int *)(param_3 + local_40 * 4) + (int)param_1) * 4) =
                     local_30;
              }
              if (*param_4 < local_30) {
                *param_4 = local_30;
              }
              local_24 = local_24 * iVar11;
              iVar7 = *piVar2;
              param_1 = (int *)((int)param_1 + 1);
            } while ((int)param_1 < iVar7);
          }
          local_40 = local_40 + 1;
        }
        local_3c = local_3c + 1;
      } while (local_3c < piVar2[1]);
    }
  }
  else if ((param_1[3] == 2) && (local_3c = 0, 0 < param_1[1])) {
    do {
      if ((param_3 == 0) || (*(int *)(piVar2[2] + local_3c * 4) != 0)) {
        local_20 = 0;
        param_1 = (int *)0x0;
        iVar11 = *piVar2;
        if (0 < iVar11) {
          do {
            uVar5 = *(uint *)(piVar2[8] + (int)(iVar11 * local_3c + (int)param_1) * 4);
            uVar8 = (int)uVar5 >> 0x1f;
            uVar8 = (uVar5 ^ uVar8) - uVar8;
            iVar7 = 0;
            for (uVar5 = (uVar8 ^ (int)uVar8 >> 0x1f) - ((int)uVar8 >> 0x1f); uVar5 != 0;
                uVar5 = uVar5 >> 1) {
              iVar7 = iVar7 + 1;
            }
            iVar9 = uVar8 << (-((char)iVar7 + -0x1f) & 0x1fU);
            if ((uVar3 == 0) || (iVar9 == 0)) {
              lVar1 = 0;
            }
            else {
              local_30 = iVar7 + 1 + local_1c;
              lVar1 = (longlong)iVar9 * (longlong)(int)uVar3;
            }
            uVar5 = Huffman_BuildCombinedNode(local_30,(uint)((ulonglong)lVar1 >> 0x20),local_28);
            uVar5 = Huffman_BuildCombinedNode(local_30,uVar5,local_20);
            if (piVar2[7] != 0) {
              local_20 = local_30;
            }
            if (param_3 == 0) {
              piVar4[(int)(iVar11 * local_40 + (int)param_1)] = uVar5;
              *(int *)((int)_Memory + (int)(*piVar2 * local_40 + (int)param_1) * 4) = local_30;
            }
            else {
              piVar4[(int)(*(int *)(param_3 + local_40 * 4) * iVar11 + (int)param_1)] = uVar5;
              *(int *)((int)_Memory +
                      (int)(*piVar2 * *(int *)(param_3 + local_40 * 4) + (int)param_1) * 4) =
                   local_30;
            }
            if (*param_4 < local_30) {
              *param_4 = local_30;
            }
            iVar11 = *piVar2;
            param_1 = (int *)((int)param_1 + 1);
          } while ((int)param_1 < iVar11);
        }
        local_40 = local_40 + 1;
      }
      local_3c = local_3c + 1;
    } while (local_3c < piVar2[1]);
  }
  iVar11 = 0;
  if (0 < *piVar2 * param_2) {
    piVar6 = piVar4;
    do {
      iVar7 = *(int *)(((int)_Memory - (int)piVar4) + (int)piVar6);
      if (iVar7 < *param_4) {
        *piVar6 = *piVar6 >> ((char)*param_4 - (char)iVar7 & 0x1fU);
      }
      iVar11 = iVar11 + 1;
      piVar6 = piVar6 + 1;
    } while (iVar11 < *piVar2 * param_2);
  }
  _free(_Memory);
  return piVar4;
}

