
uint MeshArchive_LoadComplete(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int *unaff_ESI;
  longlong lVar7;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  iVar1 = unaff_ESI[2];
  iVar2 = unaff_ESI[3];
  iVar5 = unaff_ESI[0x11];
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  local_4 = 0;
  (*(code *)unaff_ESI[0x47])(*unaff_ESI,0,0,2);
  iVar3 = (*(code *)unaff_ESI[0x49])();
  unaff_ESI[4] = iVar3;
  unaff_ESI[5] = iVar3 >> 0x1f;
  unaff_ESI[2] = iVar3;
  unaff_ESI[3] = iVar3 >> 0x1f;
  lVar7 = MeshArchive_ReadChunks(&local_10);
  iVar3 = (int)((ulonglong)lVar7 >> 0x20);
  if ((iVar3 == 0 || lVar7 < 0) && (lVar7 < 0)) {
    return (uint)lVar7;
  }
  iVar4 = AthenaList_Sort_14();
  AthenaList_FreeAll(&local_10);
  if (iVar4 == iVar5) {
    iVar5 = MeshArchive_BuildTree
                      (unaff_ESI,0,0,(uint)lVar7,iVar3,(uint)(lVar7 + 1),
                       (int)((ulonglong)(lVar7 + 1) >> 0x20),iVar5,0);
    if (iVar5 == 0) goto LAB_00478e42;
  }
  else {
    iVar5 = MeshArchive_BuildTree
                      (unaff_ESI,0,0,0,0,(uint)(lVar7 + 1),(int)((ulonglong)(lVar7 + 1) >> 0x20),
                       iVar5,0);
    if (-1 < iVar5) {
LAB_00478e42:
      MeshArchive_LoadAll(iVar1,iVar2);
      uVar6 = MeshArchive_SeekRead((int)unaff_ESI,0,0);
      return uVar6;
    }
  }
  return 0xffffff80;
}

