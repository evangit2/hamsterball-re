
void __thiscall Collision_TraverseSpatialTree(void *this,void *param_1,void *param_2)

{
  ushort uVar1;
  int iVar2;
  int iVar3;
  void *this_00;
  int iVar4;
  int iVar5;
  float *pfVar6;
  
  iVar2 = AthenaList_NextIndex((int)this + 0x18);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x20) = 0;
  if (*(int *)((int)this + 0x1c) < 1) {
    this_00 = (void *)0x0;
  }
  else {
    this_00 = (void *)**(undefined4 **)((int)this + 0x424);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x20) = 1;
  }
  while (this_00 != (void *)0x0) {
    Collision_TraverseSpatialTree(this_00,param_1,param_2);
    iVar3 = *(int *)((int)this + iVar2 * 4 + 0x20);
    if (*(int *)((int)this + 0x1c) <= iVar3) break;
    this_00 = *(void **)(*(int *)((int)this + 0x424) + iVar3 * 4);
    *(int *)((int)this + iVar2 * 4 + 0x20) = iVar3 + 1;
  }
  iVar3 = AthenaList_NextIndex(*(int *)((int)this + 8) + 0x2c);
  iVar2 = *(int *)((int)this + 8);
  *(undefined4 *)(iVar2 + 0x34 + iVar3 * 4) = 0;
  if (*(int *)(iVar2 + 0x30) < 1) {
    iVar4 = 0;
  }
  else {
    iVar4 = **(int **)(iVar2 + 0x438);
    *(undefined4 *)(iVar2 + 0x34 + iVar3 * 4) = 1;
  }
  do {
    if (iVar4 == 0) {
      return;
    }
    iVar2 = AthenaList_NextIndex(iVar4 + 0xc);
    *(undefined4 *)(iVar4 + 0x14 + iVar2 * 4) = 0;
    if (*(int *)(iVar4 + 0x10) < 1) {
      pfVar6 = (float *)0x0;
    }
    else {
      pfVar6 = (float *)**(undefined4 **)(iVar4 + 0x418);
      *(undefined4 *)(iVar4 + 0x14 + iVar2 * 4) = 1;
    }
    while (pfVar6 != (float *)0x0) {
      iVar5 = 3;
      do {
        uVar1 = AABB_ContainsPoint(param_1,*pfVar6,pfVar6[1],pfVar6[2]);
        if ((char)uVar1 != '\0') {
          AthenaList_Append(param_2,(int)pfVar6);
        }
        pfVar6 = pfVar6 + 8;
        iVar5 = iVar5 + -1;
      } while (iVar5 != 0);
      iVar5 = *(int *)(iVar4 + 0x14 + iVar2 * 4);
      if (*(int *)(iVar4 + 0x10) <= iVar5) break;
      pfVar6 = *(float **)(*(int *)(iVar4 + 0x418) + iVar5 * 4);
      *(int *)(iVar4 + 0x14 + iVar2 * 4) = iVar5 + 1;
    }
    iVar2 = *(int *)((int)this + 8);
    iVar5 = *(int *)(iVar2 + 0x34 + iVar3 * 4);
    if (*(int *)(iVar2 + 0x30) <= iVar5) {
      return;
    }
    iVar4 = *(int *)(*(int *)(iVar2 + 0x438) + iVar5 * 4);
    *(int *)(iVar2 + 0x34 + iVar3 * 4) = iVar5 + 1;
  } while( true );
}

