
undefined4 __cdecl Vorbis_DecodeAudioFrame(int *param_1,undefined4 *param_2,int param_3)

{
  uint *puVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  undefined4 uVar9;
  
  iVar2 = *(int *)(param_1[0x12] + 0x48);
  iVar3 = *(int *)(param_1[0x12] + 4);
  iVar4 = *(int *)(iVar3 + 0x1c);
  puVar1 = (uint *)(param_1 + 1);
  Vorbis_FlushDecodeBuffer((int)param_1);
  Huffman_StreamInit(puVar1,(undefined4 *)*param_2);
  uVar5 = BitStream_ReadBits(puVar1,1);
  if (uVar5 != 0) {
    return 0xffffff79;
  }
  uVar5 = BitStream_ReadBits(puVar1,*(int *)(iVar2 + 8));
  if (uVar5 == 0xffffffff) {
    return 0xffffff78;
  }
  param_1[0xb] = uVar5;
  iVar7 = **(int **)(iVar4 + 0x20 + uVar5 * 4);
  param_1[8] = iVar7;
  if (iVar7 == 0) {
    param_1[7] = 0;
    param_1[9] = 0;
  }
  else {
    uVar6 = BitStream_ReadBits(puVar1,1);
    param_1[7] = uVar6;
    uVar6 = BitStream_ReadBits(puVar1,1);
    param_1[9] = uVar6;
    if (uVar6 == 0xffffffff) {
      return 0xffffff78;
    }
  }
  param_1[0xe] = param_2[4];
  param_1[0xf] = param_2[5];
  uVar6 = param_2[6];
  iVar7 = param_2[7];
  param_1[0x10] = uVar6 - 3;
  param_1[0x11] = iVar7 - (uint)(uVar6 < 3);
  param_1[0xc] = param_2[3];
  if (param_3 != 0) {
    param_1[10] = *(int *)(iVar4 + param_1[8] * 4);
    iVar7 = Pool_Alloc((int)param_1,*(int *)(iVar3 + 4) << 2);
    *param_1 = iVar7;
    iVar7 = 0;
    if (0 < *(int *)(iVar3 + 4)) {
      do {
        iVar8 = Pool_Alloc((int)param_1,param_1[10] << 2);
        *(int *)(*param_1 + iVar7 * 4) = iVar8;
        iVar7 = iVar7 + 1;
      } while (iVar7 < *(int *)(iVar3 + 4));
    }
    uVar9 = (**(code **)((&PTR_PTR_004fa7d8)
                         [*(int *)(iVar4 + 0x120 +
                                  *(int *)(*(int *)(iVar4 + 0x20 + uVar5 * 4) + 0xc) * 4)] + 0x10))
                      (param_1,*(undefined4 *)(*(int *)(iVar2 + 0xc) + uVar5 * 4));
    return uVar9;
  }
  param_1[10] = 0;
  *param_1 = 0;
  return 0;
}

