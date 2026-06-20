
undefined4 __cdecl MeshArchive_SeekRead(int param_1,uint param_2,int param_3)

{
  byte bVar1;
  int *piVar2;
  int iVar3;
  undefined3 extraout_var;
  uint *puVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  bool bVar10;
  longlong lVar11;
  int local_48;
  uint local_44;
  int local_40 [8];
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  uint local_10;
  uint local_c;
  undefined4 local_8;
  undefined4 local_4;
  int *piVar4;
  
  local_40[0] = 0;
  local_40[1] = 0;
  local_40[2] = 0;
  local_40[3] = 0;
  local_20 = 0;
  local_1c = 0;
  local_18 = 0;
  local_14 = 0;
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  local_4 = 0;
  if (*(int *)(param_1 + 0x40) < 2) {
    return 0xffffff7d;
  }
  if (*(int *)(param_1 + 4) == 0) {
    return 0xffffff76;
  }
  if (((param_3 < 0) || (*(int *)(param_1 + 0x14) < param_3)) ||
     ((*(int *)(param_1 + 0x14) <= param_3 && (*(uint *)(param_1 + 0x10) < param_2)))) {
    return 0xffffff7d;
  }
  *(undefined4 *)(param_1 + 0x38) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x3c) = 0xffffffff;
  BitStream_InitWithBytes(*(undefined4 **)(param_1 + 0x60),*(undefined4 *)(param_1 + 0x44));
  Vorbis_ResetDecoder(param_1 + 0x68);
  MeshArchive_SetPosition(param_2,param_3);
  local_48 = 0;
  local_44 = 0;
  piVar2 = (int *)WebClient_InitResponse(*(undefined4 *)(param_1 + 0x44));
  piVar4 = piVar2;
  iVar8 = 0;
LAB_004788d2:
  while ((2 < *(int *)(param_1 + 0x40) && (iVar3 = D3DX_Uninit(), 0 < iVar3))) {
    iVar3 = *(int *)(param_1 + 0x48) * 0x20 + *(int *)(param_1 + 0x30);
    if (*(int *)(iVar3 + 0x1c) == 0) {
      D3DX_Uninit();
      break;
    }
    local_48 = Vorbis_LookupMappingValue(iVar3,&local_20);
    if (local_48 < 0) {
      D3DX_Uninit();
      local_48 = 0;
    }
    else if (piVar4 == (int *)0x0) {
      if (iVar8 != 0) {
        local_44 = local_44 + (local_48 + iVar8 >> 2);
      }
    }
    else {
      D3DX_Uninit();
    }
    iVar8 = local_48;
    if ((local_10 & local_c) != 0xffffffff) {
      iVar8 = *(int *)(param_1 + 0x48);
      iVar3 = *(int *)(param_1 + 0x2c);
      puVar5 = (uint *)(iVar8 * 0x10 + iVar3);
      uVar9 = local_10 - *puVar5;
      iVar7 = (local_c - *(int *)(iVar8 * 0x10 + 4 + iVar3)) - (uint)(local_10 < *puVar5);
      if ((iVar7 < 1) && (iVar7 < 0)) {
        uVar9 = 0;
        iVar7 = 0;
      }
      if (0 < iVar8) {
        puVar5 = (uint *)(iVar3 + 8);
        do {
          bVar10 = CARRY4(uVar9,*puVar5);
          uVar9 = uVar9 + *puVar5;
          iVar7 = iVar7 + puVar5[1] + (uint)bVar10;
          puVar5 = puVar5 + 4;
          iVar8 = iVar8 + -1;
        } while (iVar8 != 0);
      }
      *(uint *)(param_1 + 0x38) = uVar9 - local_44;
      *(uint *)(param_1 + 0x3c) = (iVar7 - ((int)local_44 >> 0x1f)) - (uint)(uVar9 < local_44);
LAB_00478b42:
      BitStream_dtor(&local_20);
      AthenaList_FreeAll(local_40);
      StreamWriter_Destroy(piVar2);
      *(undefined4 *)(param_1 + 0x50) = 0;
      *(undefined4 *)(param_1 + 0x54) = 0;
      *(undefined4 *)(param_1 + 0x58) = 0;
      *(undefined4 *)(param_1 + 0x5c) = 0;
      return 0;
    }
  }
  if (iVar8 == 0) {
    lVar11 = FileStream_SeekRead(local_40,0xffffffff,-1);
    if (-1 < lVar11) {
      if (*(int *)(param_1 + 0x40) < 3) {
LAB_004789e7:
        iVar8 = AthenaList_Sort_14();
        iVar3 = 0;
        *(int *)(param_1 + 0x44) = iVar8;
        if (0 < *(int *)(param_1 + 0x1c)) {
          piVar4 = *(int **)(param_1 + 0x28);
          do {
            if (*piVar4 == iVar8) break;
            iVar3 = iVar3 + 1;
            piVar4 = piVar4 + 1;
          } while (iVar3 < *(int *)(param_1 + 0x1c));
        }
        if (iVar3 == *(int *)(param_1 + 0x1c)) {
          BitStream_dtor(&local_20);
          AthenaList_FreeAll(local_40);
          *(undefined4 *)(param_1 + 0x38) = 0xffffffff;
          *(undefined4 *)(param_1 + 0x3c) = 0xffffffff;
          StreamWriter_Destroy(piVar2);
          Vorbis_FreeDecoder((undefined4 *)(param_1 + 0x68));
          Vorbis_FreeDecodeState((undefined4 *)(param_1 + 0xb8));
          *(undefined4 *)(param_1 + 0x40) = 2;
          return 0xffffff77;
        }
        *(int *)(param_1 + 0x48) = iVar3;
        BitStream_InitWithBytes(*(undefined4 **)(param_1 + 0x60),iVar8);
        BitStream_InitWithBytes(piVar2,*(undefined4 *)(param_1 + 0x44));
        *(undefined4 *)(param_1 + 0x40) = 3;
        iVar8 = local_48;
      }
      else {
        iVar3 = AthenaList_Sort_14();
        if (*(int *)(param_1 + 0x44) != iVar3) {
          MeshArchive_ReleaseBuffers();
          StreamWriter_Destroy(piVar2);
        }
        if (*(int *)(param_1 + 0x40) < 3) goto LAB_004789e7;
      }
      OggStream_CloneState(local_40 + 4,local_40);
      bVar1 = StreamReader_ReadSyncFlag(local_40);
      piVar4 = (int *)CONCAT31(extraout_var,bVar1);
      AthenaList_SortMerge(*(int **)(param_1 + 0x60),local_40);
      AthenaList_SortMerge(piVar2,local_40 + 4);
      goto LAB_004788d2;
    }
    lVar11 = BitStream_ReadValue(param_1,-1);
    uVar6 = (undefined4)lVar11;
    *(int *)(param_1 + 0x3c) = (int)((ulonglong)lVar11 >> 0x20);
  }
  else {
    uVar6 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x3c) = 0xffffffff;
  }
  *(undefined4 *)(param_1 + 0x38) = uVar6;
  goto LAB_00478b42;
}

