
void __thiscall Scene_CollectByNameFilter(void *this,byte *param_1,void *param_2)

{
  byte bVar1;
  byte *pbVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  byte *pbVar6;
  void *this_00;
  int iVar7;
  byte *pbVar8;
  bool bVar9;
  
  if (*(char *)((int)this + 0x430) != '\0') {
    iVar4 = AthenaList_NextIndex((int)this + 0x18);
    *(undefined4 *)((int)this + iVar4 * 4 + 0x20) = 0;
    if (*(int *)((int)this + 0x1c) < 1) {
      this_00 = (void *)0x0;
    }
    else {
      this_00 = (void *)**(undefined4 **)((int)this + 0x424);
      *(undefined4 *)((int)this + iVar4 * 4 + 0x20) = 1;
    }
    while (this_00 != (void *)0x0) {
      Scene_CollectByNameFilter(this_00,param_1,param_2);
      iVar7 = *(int *)((int)this + iVar4 * 4 + 0x20);
      if (*(int *)((int)this + 0x1c) <= iVar7) break;
      this_00 = *(void **)(*(int *)((int)this + 0x424) + iVar7 * 4);
      *(int *)((int)this + iVar4 * 4 + 0x20) = iVar7 + 1;
    }
  }
  iVar5 = AthenaList_NextIndex(*(int *)((int)this + 8) + 0x2c);
  iVar4 = *(int *)((int)this + 8);
  iVar7 = 0;
  *(undefined4 *)(iVar4 + 0x34 + iVar5 * 4) = 0;
  if (0 < *(int *)(iVar4 + 0x30)) {
    iVar7 = **(int **)(iVar4 + 0x438);
    *(undefined4 *)(iVar4 + 0x34 + iVar5 * 4) = 1;
  }
  do {
    if (iVar7 == 0) {
      return;
    }
    pbVar2 = *(byte **)(iVar7 + 0x864);
    bVar9 = false;
    if (pbVar2 == (byte *)0x0) {
LAB_004603f7:
      if (param_1 != (byte *)0x0) goto LAB_004603ff;
LAB_00460403:
      AthenaList_Append(param_2,iVar7);
    }
    else {
      pbVar6 = pbVar2;
      pbVar8 = param_1;
      if (param_1 != (byte *)0x0) {
        do {
          bVar1 = *pbVar6;
          bVar9 = bVar1 < *pbVar8;
          if (bVar1 != *pbVar8) {
LAB_004603e4:
            iVar4 = (1 - (uint)bVar9) - (uint)(bVar9 != 0);
            goto LAB_004603e9;
          }
          if (bVar1 == 0) break;
          bVar1 = pbVar6[1];
          bVar9 = bVar1 < pbVar8[1];
          if (bVar1 != pbVar8[1]) goto LAB_004603e4;
          pbVar6 = pbVar6 + 2;
          pbVar8 = pbVar8 + 2;
        } while (bVar1 != 0);
        iVar4 = 0;
LAB_004603e9:
        bVar9 = true;
        if (iVar4 != 0) {
          bVar9 = false;
        }
      }
      if (pbVar2 == (byte *)0x0) goto LAB_004603f7;
LAB_004603ff:
      if (bVar9) goto LAB_00460403;
    }
    iVar4 = *(int *)((int)this + 8);
    iVar3 = *(int *)(iVar4 + 0x34 + iVar5 * 4);
    if (*(int *)(iVar4 + 0x30) <= iVar3) {
      return;
    }
    iVar7 = *(int *)(*(int *)(iVar4 + 0x438) + iVar3 * 4);
    *(int *)(iVar4 + 0x34 + iVar5 * 4) = iVar3 + 1;
  } while( true );
}

