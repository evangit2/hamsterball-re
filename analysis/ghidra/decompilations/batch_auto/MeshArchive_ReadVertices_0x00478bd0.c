
uint __cdecl MeshArchive_ReadVertices(int param_1,short *param_2,int param_3,undefined4 *param_4)

{
  int iVar1;
  short sVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  short *psVar8;
  int local_c;
  int local_4;
  
  if (*(int *)(param_1 + 0x40) < 2) {
    return 0xffffff7d;
  }
  do {
    if ((*(int *)(param_1 + 0x40) == 4) &&
       (uVar3 = Vorbis_GetPCMOutput(param_1 + 0x68,&local_4), uVar3 != 0)) {
      if ((int)uVar3 < 1) {
        return uVar3;
      }
      if ((*(int *)(param_1 + 4) == 0) || (*(int *)(param_1 + 0x40) < 3)) {
        iVar4 = *(int *)(param_1 + 0x30);
      }
      else {
        iVar4 = *(int *)(param_1 + 0x48) * 0x20 + *(int *)(param_1 + 0x30);
      }
      iVar4 = *(int *)(iVar4 + 4);
      if (iVar4 == 1) {
        uVar5 = param_3 / 2;
      }
      else {
        uVar5 = (int)(param_3 + (param_3 >> 0x1f & 3U)) >> 2;
      }
      if ((int)uVar5 < (int)uVar3) {
        uVar3 = uVar5;
      }
      local_c = 0;
      if (0 < iVar4) {
        do {
          iVar1 = *(int *)(local_4 + local_c * 4);
          if (0 < (int)uVar3) {
            iVar7 = 0;
            psVar8 = param_2;
            do {
              iVar6 = *(int *)(iVar1 + iVar7 * 4) >> 9;
              sVar2 = (short)iVar6;
              *psVar8 = (sVar2 - ((-0x8001 < iVar6) - 1 & sVar2 + 0x8000U)) -
                        ((iVar6 < 0x8000) - 1 & sVar2 + 0x8001U);
              psVar8 = psVar8 + iVar4;
              iVar7 = iVar7 + 1;
            } while (iVar7 < (int)uVar3);
          }
          local_c = local_c + 1;
          param_2 = param_2 + 1;
        } while (local_c < iVar4);
      }
      Vorbis_SkipSamples(param_1 + 0x68,uVar3);
      *(uint *)(param_1 + 0x3c) =
           *(int *)(param_1 + 0x3c) + ((int)uVar3 >> 0x1f) +
           (uint)CARRY4(*(uint *)(param_1 + 0x38),uVar3);
      *(uint *)(param_1 + 0x38) = *(uint *)(param_1 + 0x38) + uVar3;
      if (param_4 != (undefined4 *)0x0) {
        *param_4 = *(undefined4 *)(param_1 + 0x48);
      }
      return iVar4 * uVar3 * 2;
    }
    uVar3 = MeshArchive_LoadFrame(1,1);
    if (uVar3 == 0xfffffffe) {
      return 0;
    }
  } while (0 < (int)uVar3);
  return uVar3;
}

