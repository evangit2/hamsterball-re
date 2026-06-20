
/* WARNING: Removing unreachable block (ram,0x00477c17) */
/* WARNING: Removing unreachable block (ram,0x00477c80) */

undefined4 __cdecl
MeshArchive_BuildTree
          (int *param_1,undefined4 param_2,undefined4 param_3,uint param_4,int param_5,uint param_6,
          int param_7,int param_8,int param_9)

{
  longlong lVar1;
  longlong lVar2;
  longlong lVar3;
  int iVar4;
  int iVar5;
  void *pvVar6;
  bool bVar7;
  undefined8 uVar8;
  longlong lVar9;
  undefined4 local_30;
  undefined4 local_2c;
  uint local_28;
  int local_24;
  int local_1c;
  undefined4 local_10;
  uint local_c;
  undefined4 local_8;
  uint local_4;
  
  lVar9 = CONCAT44(param_5,param_4);
  local_28 = param_6;
  local_24 = param_7;
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  local_4 = 0;
  lVar2 = CONCAT44(param_7,param_6);
  lVar1 = lVar9;
  if ((param_5 <= param_7) &&
     ((lVar1 = CONCAT44(param_5,param_4), lVar3 = CONCAT44(param_7,param_6), param_5 < param_7 ||
      (lVar1 = lVar9, lVar2 = CONCAT44(param_7,param_6), lVar3 = CONCAT44(param_7,param_6),
      param_4 < param_6)))) {
    do {
      local_1c = (int)((ulonglong)lVar1 >> 0x20);
      uVar8 = CONCAT44(local_1c,param_4);
      bVar7 = -1 < (int)((local_24 - local_1c) - (uint)(local_28 < param_4));
      if ((local_24 - local_1c != (uint)(local_28 < param_4) && bVar7) ||
         ((bVar7 && (0x3ff < local_28 - param_4)))) {
        uVar8 = __alldiv(local_28 + param_4,local_24 + local_1c + (uint)CARRY4(local_28,param_4),2,0
                        );
        local_1c = (int)((ulonglong)uVar8 >> 0x20);
      }
      if (*param_1 != 0) {
        (*(code *)param_1[0x47])(*param_1,uVar8,0);
        *(undefined8 *)(param_1 + 2) = uVar8;
        Pool_Reset(param_1[6]);
      }
      lVar9 = FileStream_SeekRead(&local_10,0xffffffff,-1);
      if (lVar9 == -0x80) {
        return 0xffffff80;
      }
      lVar2 = lVar3;
      if ((lVar9 < 0) || (iVar4 = AthenaList_Sort_14(), iVar4 != param_8)) {
        local_24 = local_1c;
        local_28 = (uint)uVar8;
        if (-1 < lVar9) {
          lVar2 = lVar9;
        }
      }
      else {
        lVar1 = lVar9 + CONCAT44(((int)local_4 >> 0x1f) + ((int)local_c >> 0x1f) +
                                 (uint)CARRY4(local_4,local_c),local_4 + local_c);
      }
      param_4 = (uint)lVar1;
      AthenaList_FreeAll(&local_10);
      lVar3 = lVar2;
    } while (lVar1 < CONCAT44(local_24,local_28));
  }
  local_2c = (undefined4)((ulonglong)lVar2 >> 0x20);
  local_30 = (undefined4)lVar2;
  if (*param_1 != 0) {
    (*(code *)param_1[0x47])(*param_1,lVar2,0);
    *(longlong *)(param_1 + 2) = lVar2;
    Pool_Reset(param_1[6]);
  }
  lVar9 = FileStream_SeekRead(&local_10,0xffffffff,-1);
  if (lVar9 != -0x80) {
    if ((lVar1 < CONCAT44(param_7,param_6)) && (-1 < lVar9)) {
      iVar4 = param_9 + 1;
      iVar5 = AthenaList_Sort_14();
      iVar4 = MeshArchive_BuildTree
                        (param_1,local_30,local_2c,param_1[2],param_1[3],param_6,param_7,iVar5,iVar4
                        );
      AthenaList_FreeAll(&local_10);
      if (iVar4 == -0x80) {
        return 0xffffff80;
      }
    }
    else {
      AthenaList_FreeAll(&local_10);
      param_1[7] = param_9 + 1;
      pvVar6 = _malloc((param_9 + 1) * 8 + 8);
      param_1[8] = (int)pvVar6;
      pvVar6 = _malloc(param_1[7] << 2);
      iVar4 = param_1[8];
      param_1[10] = (int)pvVar6;
      *(int *)(iVar4 + 8 + param_9 * 8) = (int)lVar1;
      *(int *)(iVar4 + 0xc + param_9 * 8) = (int)((ulonglong)lVar1 >> 0x20);
    }
    iVar4 = param_1[8];
    *(undefined4 *)(iVar4 + param_9 * 8) = param_2;
    *(undefined4 *)(iVar4 + 4 + param_9 * 8) = param_3;
    *(int *)(param_1[10] + param_9 * 4) = param_8;
    return 0;
  }
  return 0xffffff80;
}

