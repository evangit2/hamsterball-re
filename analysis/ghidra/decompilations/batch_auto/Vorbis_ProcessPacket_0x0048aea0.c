
undefined4 __cdecl Vorbis_ProcessPacket(undefined4 *param_1,int param_2,undefined4 *param_3)

{
  uint uVar1;
  uint uVar2;
  undefined4 uVar3;
  short *psVar4;
  int iVar5;
  short *psVar6;
  bool bVar7;
  short local_20 [4];
  uint local_18 [6];
  
  if (param_3 != (undefined4 *)0x0) {
    Huffman_StreamInit(local_18,(undefined4 *)*param_3);
    uVar1 = BitStream_ReadBits(local_18,8);
    local_20[0] = 0;
    local_20[1] = 0;
    local_20[2] = 0;
    psVar4 = local_20;
    iVar5 = 6;
    do {
      uVar2 = BitStream_ReadBits(local_18,8);
      *(char *)psVar4 = (char)uVar2;
      psVar4 = (short *)((int)psVar4 + 1);
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);
    iVar5 = 3;
    bVar7 = true;
    psVar4 = local_20;
    psVar6 = (short *)"vorbis";
    do {
      if (iVar5 == 0) break;
      iVar5 = iVar5 + -1;
      bVar7 = *psVar4 == *psVar6;
      psVar4 = psVar4 + 1;
      psVar6 = psVar6 + 1;
    } while (bVar7);
    if (!bVar7) {
      return 0xffffff7c;
    }
    if (uVar1 == 1) {
      if ((param_3[2] != 0) && (param_1[2] == 0)) {
        uVar3 = Vorbis_ReadIDHeader();
        return uVar3;
      }
    }
    else if (uVar1 == 3) {
      if (param_1[2] != 0) {
        uVar3 = Vorbis_ReadCommentHeader(local_18);
        return uVar3;
      }
    }
    else if (((uVar1 == 5) && (param_1[2] != 0)) && (*(int *)(param_2 + 0xc) != 0)) {
      uVar3 = Vorbis_ReadSetupHeader(param_1);
      return uVar3;
    }
  }
  return 0xffffff7b;
}

