
/* WARNING: Removing unreachable block (ram,0x004781d2) */

void __cdecl MeshArchive_LoadAll(int param_1,int param_2)

{
  longlong *plVar1;
  int *piVar2;
  int *in_EAX;
  void *pvVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  bool bVar9;
  longlong lVar10;
  longlong lVar11;
  int local_50;
  uint uStack_48;
  int iStack_44;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 uStack_20;
  undefined4 uStack_1c;
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  undefined4 uStack_c;
  undefined4 uStack_8;
  undefined4 uStack_4;
  
  local_30 = 0;
  local_2c = 0;
  local_28 = 0;
  local_24 = 0;
  pvVar3 = _realloc((void *)in_EAX[0xc],in_EAX[7] << 5);
  in_EAX[0xc] = (int)pvVar3;
  pvVar3 = _realloc((void *)in_EAX[0xd],in_EAX[7] << 4);
  in_EAX[0xd] = (int)pvVar3;
  pvVar3 = _malloc(in_EAX[7] << 3);
  in_EAX[9] = (int)pvVar3;
  pvVar3 = _malloc(in_EAX[7] << 4);
  in_EAX[0xb] = (int)pvVar3;
  local_50 = 0;
  if (0 < in_EAX[7]) {
    do {
      if (local_50 == 0) {
        piVar2 = (int *)in_EAX[9];
        *piVar2 = param_1;
        piVar2[1] = param_2;
        if (*in_EAX != 0) {
          (*(code *)in_EAX[0x47])(*in_EAX,param_1,param_2,0);
          in_EAX[2] = param_1;
          in_EAX[3] = param_2;
          Pool_Reset(in_EAX[6]);
        }
      }
      else {
        iVar4 = *(int *)(in_EAX[8] + local_50 * 8);
        iVar8 = *(int *)(in_EAX[8] + 4 + local_50 * 8);
        if (*in_EAX != 0) {
          (*(code *)in_EAX[0x47])(*in_EAX,iVar4,iVar8,0);
          in_EAX[2] = iVar4;
          in_EAX[3] = iVar8;
          Pool_Reset(in_EAX[6]);
        }
        iVar4 = MeshArchive_LoadSubmesh
                          (in_EAX,(undefined4 *)(local_50 * 0x20 + in_EAX[0xc]),
                           (int *)(local_50 * 0x10 + in_EAX[0xd]),(int *)0x0);
        if (iVar4 < 0) {
          iVar4 = in_EAX[9];
          *(undefined4 *)(iVar4 + local_50 * 8) = 0xffffffff;
          *(undefined4 *)(iVar4 + 4 + local_50 * 8) = 0xffffffff;
        }
        else {
          iVar4 = in_EAX[9];
          *(int *)(iVar4 + local_50 * 8) = in_EAX[2];
          *(int *)(iVar4 + 4 + local_50 * 8) = in_EAX[3];
        }
      }
      iVar4 = local_50 * 8;
      if ((*(uint *)(iVar4 + in_EAX[9]) & *(uint *)(iVar4 + 4 + in_EAX[9])) != 0xffffffff) {
        uStack_48 = 0;
        iStack_44 = 0;
        lVar11 = 0;
        iVar8 = -1;
        BitStream_InitWithBytes
                  ((undefined4 *)in_EAX[0x18],*(undefined4 *)(in_EAX[10] + local_50 * 4));
        uStack_20 = 0;
        uStack_1c = 0;
        uStack_18 = 0;
        uStack_14 = 0;
        uStack_10 = 0;
        uStack_c = 0;
        uStack_8 = 0;
        uStack_4 = 0;
        lVar10 = FileStream_SeekRead(&local_30,0xffffffff,-1);
        if (-1 < lVar10) {
          do {
            lVar11 = CONCAT44(iStack_44,uStack_48);
            iVar5 = AthenaList_Sort_14();
            if (iVar5 != *(int *)(in_EAX[10] + local_50 * 4)) break;
            lVar10 = CRT_FltControl87();
            AthenaList_SortMerge((int *)in_EAX[0x18],&local_30);
            iVar5 = D3DX_Uninit();
            while (lVar11 = CONCAT44(iStack_44,uStack_48), iVar5 != 0) {
              iVar6 = iVar8;
              if ((0 < iVar5) &&
                 (iVar6 = Vorbis_LookupMappingValue(local_50 * 0x20 + in_EAX[0xc],&uStack_20),
                 iVar8 != -1)) {
                uVar7 = iVar6 + iVar8 >> 2;
                bVar9 = CARRY4(uStack_48,uVar7);
                uStack_48 = uStack_48 + uVar7;
                iStack_44 = iStack_44 + (iVar6 + iVar8 >> 0x1f) + (uint)bVar9;
              }
              iVar5 = D3DX_Uninit();
              iVar8 = iVar6;
            }
            BitStream_dtor(&uStack_20);
            if (lVar10 != -1) {
              lVar11 = lVar10 - lVar11;
              break;
            }
            uStack_20 = 0;
            uStack_1c = 0;
            uStack_18 = 0;
            uStack_14 = 0;
            uStack_10 = 0;
            uStack_c = 0;
            uStack_8 = 0;
            uStack_4 = 0;
            lVar10 = FileStream_SeekRead(&local_30,0xffffffff,-1);
          } while ((0 < (int)((ulonglong)lVar10 >> 0x20)) || (-1 < lVar10));
          iStack_44 = (int)((ulonglong)lVar11 >> 0x20);
          if ((iStack_44 < 1) && (lVar11 < 0)) {
            lVar11 = 0;
          }
        }
        iStack_44 = (int)((ulonglong)lVar11 >> 0x20);
        uStack_48 = (uint)lVar11;
        iVar8 = in_EAX[0xb];
        *(uint *)(iVar8 + local_50 * 0x10) = uStack_48;
        *(int *)(iVar8 + 4 + local_50 * 0x10) = iStack_44;
      }
      iVar8 = *(int *)(iVar4 + 8 + in_EAX[8]);
      iVar4 = *(int *)(iVar4 + 0xc + in_EAX[8]);
      if (*in_EAX != 0) {
        (*(code *)in_EAX[0x47])(*in_EAX,iVar8,iVar4,0);
        in_EAX[2] = iVar8;
        in_EAX[3] = iVar4;
        Pool_Reset(in_EAX[6]);
      }
      lVar11 = MeshArchive_ReadChunks(&local_30);
      while (-1 < lVar11) {
        do {
          lVar10 = CRT_FltControl87();
          if (lVar10 != -1) {
            plVar1 = (longlong *)(local_50 * 0x10 + in_EAX[0xb]);
            lVar11 = CRT_FltControl87();
            plVar1[1] = lVar11 - *plVar1;
            goto LAB_004782ee;
          }
          *(longlong *)(in_EAX + 2) = lVar11;
          lVar11 = MeshArchive_ReadChunks(&local_30);
        } while (0 < (int)((ulonglong)lVar11 >> 0x20));
      }
      D3DResourcePool_Release((undefined4 *)(local_50 * 0x20 + in_EAX[0xc]));
      MeshGroup_dtor((int *)(local_50 * 0x10 + in_EAX[0xd]));
LAB_004782ee:
      local_50 = local_50 + 1;
    } while (local_50 < in_EAX[7]);
  }
  AthenaList_FreeAll(&local_30);
  return;
}

