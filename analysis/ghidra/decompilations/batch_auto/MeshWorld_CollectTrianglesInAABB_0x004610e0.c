
void __thiscall MeshWorld_CollectTrianglesInAABB(void *this,void *param_1,void *param_2)

{
  int iVar1;
  ushort uVar2;
  int iVar3;
  int iVar4;
  void *this_00;
  int iVar5;
  int iVar6;
  float *pfVar7;
  float *pfVar8;
  float *pfVar9;
  float *pfVar10;
  bool bVar11;
  int local_14;
  
  iVar3 = AthenaList_NextIndex((int)this + 0x18);
  *(undefined4 *)((int)this + iVar3 * 4 + 0x20) = 0;
  if (*(int *)((int)this + 0x1c) < 1) {
    this_00 = (void *)0x0;
  }
  else {
    this_00 = (void *)**(undefined4 **)((int)this + 0x424);
    *(undefined4 *)((int)this + iVar3 * 4 + 0x20) = 1;
  }
  while (this_00 != (void *)0x0) {
    MeshWorld_CollectTrianglesInAABB(this_00,param_1,param_2);
    iVar4 = *(int *)((int)this + iVar3 * 4 + 0x20);
    if (*(int *)((int)this + 0x1c) <= iVar4) break;
    this_00 = *(void **)(*(int *)((int)this + 0x424) + iVar4 * 4);
    *(int *)((int)this + iVar3 * 4 + 0x20) = iVar4 + 1;
  }
  iVar4 = AthenaList_NextIndex(*(int *)((int)this + 8) + 0x2c);
  iVar3 = *(int *)((int)this + 8);
  *(undefined4 *)(iVar3 + 0x34 + iVar4 * 4) = 0;
  if (*(int *)(iVar3 + 0x30) < 1) {
    iVar6 = 0;
  }
  else {
    iVar6 = **(int **)(iVar3 + 0x438);
    *(undefined4 *)(iVar3 + 0x34 + iVar4 * 4) = 1;
  }
  do {
    if (iVar6 == 0) {
      return;
    }
    if ((*(char *)(iVar6 + 0x863) == '\0') && (*(char *)(iVar6 + 0x85e) == '\0')) {
      iVar3 = AthenaList_NextIndex(iVar6 + 0x424);
      *(undefined4 *)(iVar6 + 0x42c + iVar3 * 4) = 0;
      if (*(int *)(iVar6 + 0x428) < 1) {
        iVar5 = 0;
      }
      else {
        iVar5 = **(int **)(iVar6 + 0x830);
        *(undefined4 *)(iVar6 + 0x42c + iVar3 * 4) = 1;
      }
      while (iVar5 != 0) {
        if (*(char *)(*(int *)((int)this + 0x480) + 0x434) == '\0') {
          pfVar8 = (float *)(*(int *)(iVar5 + 8) * 0x20 + *(int *)(*(int *)((int)this + 8) + 0x448))
          ;
        }
        else {
          pfVar8 = (float *)(*(int *)(iVar5 + 0xc) * 0x20 +
                            *(int *)(*(int *)(*(int *)((int)this + 0x47c) + 0x480) + 0x440));
        }
        bVar11 = false;
        local_14 = 0;
        if (0 < *(int *)(iVar5 + 4)) {
          pfVar9 = pfVar8 + 0x10;
          do {
            if (bVar11) {
              pfVar7 = pfVar9 + -8;
              pfVar10 = pfVar9;
            }
            else {
              pfVar7 = pfVar9;
              pfVar10 = pfVar9 + -8;
            }
            bVar11 = bVar11 == false;
            uVar2 = AABB_ContainsPoint(param_1,*pfVar8,pfVar9[-0xf],pfVar9[-0xe]);
            if ((char)uVar2 != '\0') {
              AthenaList_Append(param_2,(int)pfVar8);
            }
            uVar2 = AABB_ContainsPoint(param_1,*pfVar10,pfVar10[1],pfVar10[2]);
            if ((char)uVar2 != '\0') {
              AthenaList_Append(param_2,(int)pfVar10);
            }
            uVar2 = AABB_ContainsPoint(param_1,*pfVar7,pfVar7[1],pfVar7[2]);
            if ((char)uVar2 != '\0') {
              AthenaList_Append(param_2,(int)pfVar7);
            }
            pfVar8 = pfVar8 + 8;
            pfVar9 = pfVar9 + 8;
            local_14 = local_14 + 1;
          } while (local_14 < *(int *)(iVar5 + 4));
        }
        iVar1 = *(int *)(iVar6 + 0x42c + iVar3 * 4);
        if (*(int *)(iVar6 + 0x428) <= iVar1) break;
        iVar5 = *(int *)(*(int *)(iVar6 + 0x830) + iVar1 * 4);
        *(int *)(iVar6 + 0x42c + iVar3 * 4) = iVar1 + 1;
      }
    }
    iVar3 = *(int *)((int)this + 8);
    iVar5 = *(int *)(iVar3 + 0x34 + iVar4 * 4);
    if (*(int *)(iVar3 + 0x30) <= iVar5) {
      return;
    }
    iVar6 = *(int *)(*(int *)(iVar3 + 0x438) + iVar5 * 4);
    *(int *)(iVar3 + 0x34 + iVar4 * 4) = iVar5 + 1;
  } while( true );
}

