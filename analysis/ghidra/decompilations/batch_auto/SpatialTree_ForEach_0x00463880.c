
void __thiscall
SpatialTree_ForEach(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar3 = AthenaList_NextIndex(*(int *)((int)this + 4));
  iVar1 = *(int *)((int)this + 4);
  *(undefined4 *)(iVar1 + 8 + iVar3 * 4) = 0;
  if (*(int *)(iVar1 + 4) < 1) {
    iVar4 = 0;
  }
  else {
    iVar4 = **(int **)(iVar1 + 0x40c);
    *(undefined4 *)(iVar1 + 8 + iVar3 * 4) = 1;
  }
  while( true ) {
    if (iVar4 == 0) {
      return;
    }
    (**(code **)(*(int *)this + 0x20))(iVar4,param_2,param_3);
    iVar1 = *(int *)((int)this + 4);
    iVar2 = *(int *)(iVar1 + 8 + iVar3 * 4);
    if (*(int *)(iVar1 + 4) <= iVar2) break;
    iVar4 = *(int *)(*(int *)(iVar1 + 0x40c) + iVar2 * 4);
    *(int *)(iVar1 + 8 + iVar3 * 4) = iVar2 + 1;
  }
  return;
}

