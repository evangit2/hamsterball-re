
undefined4 __cdecl MeshArchive_LoadFrame(int param_1,int param_2)

{
  void *in_EAX;
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint *puVar5;
  int *piVar6;
  uint uVar7;
  uint uVar8;
  bool bVar9;
  undefined8 uVar10;
  int local_34;
  int local_30;
  int local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  int local_14;
  uint local_10;
  uint local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  local_30 = 0;
  local_2c = 0;
  local_28 = 0;
  local_24 = 0;
  local_20 = 0;
  local_1c = 0;
  local_18 = 0;
  local_14 = 0;
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  local_4 = 0;
  local_34 = 0;
LAB_00478380:
  if (*(int *)((int)in_EAX + 0x40) == 4) {
    while( true ) {
      iVar1 = D3DX_Uninit();
      uVar7 = local_c;
      uVar8 = local_10;
      if (iVar1 < 0) {
        local_34 = 0xfffffffd;
        goto LAB_0047863b;
      }
      if (iVar1 < 1) break;
      iVar1 = Vorbis_DecodeAudioFrame((int *)((int)in_EAX + 0xb8),&local_20,1);
      if (iVar1 == 0) {
        iVar1 = (int)in_EAX + 0x68;
        iVar2 = Vorbis_GetPCMOutput(iVar1,(undefined4 *)0x0);
        if (iVar2 == 0) {
          Vorbis_ApplyWindowOverlap(iVar1,(int *)((int)in_EAX + 0xb8));
          uVar3 = Vorbis_GetPCMOutput(iVar1,(undefined4 *)0x0);
          *(int *)((int)in_EAX + 0x5c) =
               *(int *)((int)in_EAX + 0x5c) + ((int)uVar3 >> 0x1f) +
               (uint)CARRY4(*(uint *)((int)in_EAX + 0x58),uVar3);
          uVar4 = local_1c * 8;
          *(uint *)((int)in_EAX + 0x58) = *(uint *)((int)in_EAX + 0x58) + uVar3;
          uVar3 = *(uint *)((int)in_EAX + 0x50);
          *(uint *)((int)in_EAX + 0x50) = uVar3 + uVar4;
          *(uint *)((int)in_EAX + 0x54) =
               *(int *)((int)in_EAX + 0x54) + ((int)uVar4 >> 0x1f) + (uint)CARRY4(uVar3,uVar4);
          if (((uVar8 & uVar7) != 0xffffffff) && (local_14 == 0)) {
            if (*(int *)((int)in_EAX + 4) == 0) {
              local_34 = 0;
            }
            else {
              local_34 = *(int *)((int)in_EAX + 0x48);
              if (0 < local_34) {
                uVar3 = *(uint *)(local_34 * 0x10 + *(int *)((int)in_EAX + 0x2c));
                bVar9 = uVar8 < uVar3;
                uVar8 = uVar8 - uVar3;
                uVar7 = (uVar7 - *(int *)(local_34 * 0x10 + 4 + *(int *)((int)in_EAX + 0x2c))) -
                        (uint)bVar9;
              }
            }
            if (((int)uVar7 < 1) && ((int)uVar7 < 0)) {
              uVar8 = 0;
              uVar7 = 0;
            }
            uVar3 = Vorbis_GetPCMOutput(iVar1,(undefined4 *)0x0);
            uVar4 = uVar8 - uVar3;
            iVar1 = (uVar7 - ((int)uVar3 >> 0x1f)) - (uint)(uVar8 < uVar3);
            if (0 < local_34) {
              puVar5 = (uint *)(*(int *)((int)in_EAX + 0x2c) + 8);
              do {
                bVar9 = CARRY4(uVar4,*puVar5);
                uVar4 = uVar4 + *puVar5;
                iVar1 = iVar1 + puVar5[1] + (uint)bVar9;
                puVar5 = puVar5 + 4;
                local_34 = local_34 + -1;
              } while (local_34 != 0);
            }
            *(uint *)((int)in_EAX + 0x38) = uVar4;
            *(int *)((int)in_EAX + 0x3c) = iVar1;
          }
          local_34 = 1;
        }
        else {
          local_34 = 0xffffff7f;
        }
        goto LAB_0047863b;
      }
    }
  }
  if (1 < *(int *)((int)in_EAX + 0x40)) {
    if ((param_1 == 0) || (uVar10 = FileStream_SeekRead(&local_30,0xffffffff,-1), (int)uVar10 < 0))
    goto LAB_0047863b;
    uVar8 = *(uint *)((int)in_EAX + 0x50);
    uVar7 = local_2c * 8;
    iVar1 = *(int *)((int)in_EAX + 0x40);
    *(uint *)((int)in_EAX + 0x50) = uVar8 + uVar7;
    *(uint *)((int)in_EAX + 0x54) =
         *(int *)((int)in_EAX + 0x54) + ((int)uVar7 >> 0x1f) + (uint)CARRY4(uVar8,uVar7);
    if (iVar1 != 4) goto LAB_0047847b;
    iVar1 = AthenaList_Sort_14();
    if (*(int *)((int)in_EAX + 0x44) != iVar1) {
      if (param_2 != 0) {
        MeshArchive_ReleaseBuffers();
        if (*(int *)((int)in_EAX + 4) == 0) {
          D3DResourcePool_Release(*(undefined4 **)((int)in_EAX + 0x30));
          MeshGroup_dtor(*(int **)((int)in_EAX + 0x34));
        }
        goto LAB_0047846f;
      }
      goto LAB_0047863b;
    }
  }
LAB_0047846f:
  iVar1 = *(int *)((int)in_EAX + 0x40);
  if (iVar1 != 4) goto LAB_0047847b;
LAB_0047853f:
  AthenaList_SortMerge(*(int **)((int)in_EAX + 0x60),&local_30);
  goto LAB_00478380;
LAB_0047847b:
  if (iVar1 < 3) {
    if (*(int *)((int)in_EAX + 4) == 0) {
      iVar1 = MeshArchive_LoadSubmesh
                        (in_EAX,*(undefined4 **)((int)in_EAX + 0x30),*(int **)((int)in_EAX + 0x34),
                         &local_30);
      if (iVar1 == 0) {
        *(int *)((int)in_EAX + 0x48) = *(int *)((int)in_EAX + 0x48) + 1;
        goto LAB_004784f4;
      }
      goto LAB_0047863b;
    }
    iVar1 = AthenaList_Sort_14();
    iVar2 = 0;
    *(int *)((int)in_EAX + 0x44) = iVar1;
    if (0 < *(int *)((int)in_EAX + 0x1c)) {
      piVar6 = *(int **)((int)in_EAX + 0x28);
      do {
        if (*piVar6 == iVar1) break;
        iVar2 = iVar2 + 1;
        piVar6 = piVar6 + 1;
      } while (iVar2 < *(int *)((int)in_EAX + 0x1c));
    }
    if (iVar2 == *(int *)((int)in_EAX + 0x1c)) {
      local_34 = 0xffffff77;
LAB_0047863b:
      BitStream_dtor(&local_20);
      AthenaList_FreeAll(&local_30);
      return local_34;
    }
    *(int *)((int)in_EAX + 0x48) = iVar2;
    BitStream_InitWithBytes(*(undefined4 **)((int)in_EAX + 0x60),iVar1);
    *(undefined4 *)((int)in_EAX + 0x40) = 3;
  }
  else {
LAB_004784f4:
    if (*(int *)((int)in_EAX + 0x40) != 3) goto LAB_0047853f;
  }
  if (*(int *)((int)in_EAX + 4) == 0) {
    iVar1 = *(int *)((int)in_EAX + 0x30);
  }
  else {
    iVar1 = *(int *)((int)in_EAX + 0x48) * 0x20 + *(int *)((int)in_EAX + 0x30);
  }
  Vorbis_InitDecoder((int)in_EAX + 0x68,iVar1);
  Vorbis_InitDecodeState((int)in_EAX + 0x68,(undefined4 *)((int)in_EAX + 0xb8));
  *(undefined4 *)((int)in_EAX + 0x40) = 4;
  *(undefined4 *)((int)in_EAX + 0x50) = 0;
  *(undefined4 *)((int)in_EAX + 0x54) = 0;
  *(undefined4 *)((int)in_EAX + 0x58) = 0;
  *(undefined4 *)((int)in_EAX + 0x5c) = 0;
  goto LAB_0047853f;
}

