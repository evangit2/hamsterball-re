
void __cdecl Sort_IndexHeapByFloat(undefined4 *param_1,int param_2,uint param_3)

{
  float fVar1;
  float fVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  uint local_c;
  uint local_8;
  
  uVar4 = 0;
  if (param_3 != 0) {
    do {
      param_1[uVar4] = uVar4;
      uVar4 = uVar4 + 1;
    } while (uVar4 < param_3);
  }
  if (1 < param_3) {
    uVar4 = (param_3 >> 1) - 1;
    if (uVar4 != 0xffffffff) {
      local_c = (param_3 >> 1) + uVar4;
      do {
        iVar3 = param_1[uVar4];
        local_8 = uVar4;
        if (local_c < param_3) {
          uVar6 = local_c;
          do {
            iVar7 = param_1[uVar6];
            uVar5 = uVar6;
            if ((uVar6 + 1 < param_3) &&
               (fVar1 = *(float *)(param_2 + param_1[uVar6 + 1] * 4),
               fVar2 = *(float *)(param_2 + iVar7 * 4), fVar1 < fVar2 != (fVar1 == fVar2))) {
              uVar5 = uVar6 + 1;
              iVar7 = param_1[uVar6 + 1];
            }
            if (*(float *)(param_2 + iVar3 * 4) < *(float *)(param_2 + iVar7 * 4)) break;
            uVar6 = uVar5 * 2 + 1;
            param_1[local_8] = param_1[uVar5];
            local_8 = uVar5;
          } while (uVar6 < param_3);
        }
        local_c = local_c - 2;
        uVar4 = uVar4 - 1;
        param_1[local_8] = iVar3;
      } while (uVar4 != 0xffffffff);
    }
    while (param_3 = param_3 - 1, param_3 != 0xffffffff) {
      iVar3 = param_1[param_3];
      local_8 = 0;
      param_1[param_3] = *param_1;
      uVar4 = 1;
      if (1 < param_3) {
        do {
          iVar7 = param_1[uVar4];
          uVar6 = uVar4;
          if ((uVar4 + 1 < param_3) &&
             (fVar1 = *(float *)(param_2 + param_1[uVar4 + 1] * 4),
             fVar2 = *(float *)(param_2 + iVar7 * 4), fVar1 < fVar2 != (fVar1 == fVar2))) {
            uVar6 = uVar4 + 1;
            iVar7 = param_1[uVar4 + 1];
          }
          if (*(float *)(param_2 + iVar3 * 4) < *(float *)(param_2 + iVar7 * 4)) break;
          uVar4 = uVar6 * 2 + 1;
          param_1[local_8] = param_1[uVar6];
          local_8 = uVar6;
        } while (uVar4 < param_3);
      }
      param_1[local_8] = iVar3;
    }
  }
  return;
}

