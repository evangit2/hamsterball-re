
void __cdecl Deflate_BuildHuffmanTables(int *param_1)

{
  int iVar1;
  int iVar2;
  bool bVar3;
  int iVar4;
  int *piVar5;
  int iVar6;
  bool bVar7;
  bool bVar8;
  undefined4 *local_14;
  int *local_10;
  int local_8;
  
  iVar1 = param_1[0x65];
  bVar7 = param_1[0x5a] != 0;
  bVar3 = false;
  if (bVar7) {
    if ((param_1[0x5b] < param_1[0x5a]) || (0x3f < param_1[0x5b])) {
      bVar3 = true;
    }
    bVar8 = param_1[0x48] == 1;
  }
  else {
    bVar8 = param_1[0x5b] == 0;
  }
  if (!bVar8) {
    bVar3 = true;
  }
  if ((param_1[0x5c] != 0) && (param_1[0x5d] != param_1[0x5c] + -1)) {
    bVar3 = true;
  }
  if (0xd < param_1[0x5d]) {
    bVar3 = true;
  }
  if (bVar3) {
    *(undefined4 *)(*param_1 + 0x14) = 0xe;
    *(int *)(*param_1 + 0x18) = param_1[0x5a];
    *(int *)(*param_1 + 0x1c) = param_1[0x5b];
    *(int *)(*param_1 + 0x20) = param_1[0x5c];
    *(int *)(*param_1 + 0x24) = param_1[0x5d];
    (**(code **)*param_1)(param_1);
  }
  local_8 = 0;
  if (0 < param_1[0x48]) {
    local_10 = param_1 + 0x49;
    do {
      iVar2 = *(int *)(*local_10 + 4);
      piVar5 = (int *)(iVar2 * 0x100 + param_1[0x23]);
      if ((bVar7) && (*piVar5 < 0)) {
        *(undefined4 *)(*param_1 + 0x14) = 0x6f;
        *(int *)(*param_1 + 0x18) = iVar2;
        *(undefined4 *)(*param_1 + 0x1c) = 0;
        (**(code **)(*param_1 + 4))(param_1,0xffffffff);
      }
      for (iVar6 = param_1[0x5a]; iVar6 <= param_1[0x5b]; iVar6 = iVar6 + 1) {
        iVar4 = piVar5[iVar6];
        if (iVar4 < 0) {
          iVar4 = 0;
        }
        if (param_1[0x5c] != iVar4) {
          *(undefined4 *)(*param_1 + 0x14) = 0x6f;
          *(int *)(*param_1 + 0x18) = iVar2;
          *(int *)(*param_1 + 0x1c) = iVar6;
          (**(code **)(*param_1 + 4))(param_1,0xffffffff);
        }
        piVar5[iVar6] = param_1[0x5d];
      }
      local_8 = local_8 + 1;
      local_10 = local_10 + 1;
    } while (local_8 < param_1[0x48]);
  }
  if (param_1[0x5c] == 0) {
    if (bVar7) {
      *(code **)(iVar1 + 4) = Audio_DecodeLayer3Mono;
    }
    else {
      *(code **)(iVar1 + 4) = Audio_DecodeLayer3Joint;
    }
  }
  else if (bVar7) {
    *(code **)(iVar1 + 4) = Audio_DecodeScalefactors;
  }
  else {
    *(code **)(iVar1 + 4) = Audio_ReadSFBZeros;
  }
  local_8 = 0;
  if (0 < param_1[0x48]) {
    local_10 = param_1 + 0x49;
    local_14 = (undefined4 *)(iVar1 + 0x24);
    do {
      if (bVar7) {
        iVar2 = *(int *)(*local_10 + 0x18);
        if (((iVar2 < 0) || (3 < iVar2)) || (param_1[iVar2 + 0x2c] == 0)) {
          *(undefined4 *)(*param_1 + 0x14) = 0x31;
          *(int *)(*param_1 + 0x18) = iVar2;
          (**(code **)*param_1)(param_1);
        }
        piVar5 = (int *)(iVar1 + 0x38 + iVar2 * 4);
        Huffman_BuildTable((int)param_1,param_1[iVar2 + 0x2c],piVar5);
        *(int *)(iVar1 + 0x48) = *piVar5;
      }
      else if (param_1[0x5c] == 0) {
        iVar2 = *(int *)(*local_10 + 0x14);
        if (((iVar2 < 0) || (3 < iVar2)) || (param_1[iVar2 + 0x28] == 0)) {
          *(undefined4 *)(*param_1 + 0x14) = 0x31;
          *(int *)(*param_1 + 0x18) = iVar2;
          (**(code **)*param_1)(param_1);
        }
        Huffman_BuildTable((int)param_1,param_1[iVar2 + 0x28],(int *)(iVar1 + 0x38 + iVar2 * 4));
      }
      local_8 = local_8 + 1;
      local_10 = local_10 + 1;
      *local_14 = 0;
      local_14 = local_14 + 1;
    } while (local_8 < param_1[0x48]);
  }
  *(undefined1 *)(iVar1 + 0x18) = 0;
  *(undefined4 *)(iVar1 + 0x14) = 0;
  *(undefined4 *)(iVar1 + 0x10) = 0;
  *(undefined4 *)(iVar1 + 0x20) = 0;
  *(int *)(iVar1 + 0x34) = param_1[0x3f];
  return;
}

