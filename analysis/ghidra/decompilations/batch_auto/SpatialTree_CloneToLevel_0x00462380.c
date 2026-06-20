
void * __thiscall SpatialTree_CloneToLevel(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  void *pvVar4;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd07b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  pvVar2 = operator_new(0x10d0);
  local_4 = 0;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = Level_ctor(pvVar2,*(undefined4 *)((int)this + 4));
  }
  pvVar4 = *(void **)((int)pvVar2 + 0x480);
  local_4 = 0xffffffff;
  if (pvVar4 != (void *)0x0) {
    SceneObject_BaseClear((int)pvVar4);
    _free(pvVar4);
  }
  *(undefined4 *)((int)pvVar2 + 0x480) = 0;
  *(undefined1 *)((int)pvVar2 + 0x484) = 1;
  *(undefined4 *)((int)pvVar2 + 0x480) = *(undefined4 *)((int)this + 0x480);
  *(undefined4 *)((int)pvVar2 + 8) = *(undefined4 *)((int)this + 8);
  *(undefined1 *)((int)pvVar2 + 0xd) = 0;
  *(undefined1 *)((int)pvVar2 + 0x430) = *(undefined1 *)((int)this + 0x430);
  *(int *)((int)pvVar2 + 0x47c) = param_1;
  *(undefined4 *)((int)pvVar2 + 0x434) = *(undefined4 *)(param_1 + 0x434);
  *(undefined1 *)((int)pvVar2 + 0x431) = 0;
  iVar3 = AthenaList_NextIndex((int)this + 0x18);
  *(undefined4 *)((int)this + iVar3 * 4 + 0x20) = 0;
  if (*(int *)((int)this + 0x1c) < 1) {
    pvVar4 = (void *)0x0;
  }
  else {
    pvVar4 = (void *)**(undefined4 **)((int)this + 0x424);
    *(undefined4 *)((int)this + iVar3 * 4 + 0x20) = 1;
  }
  while( true ) {
    if (pvVar4 == (void *)0x0) {
      ExceptionList = local_c;
      return pvVar2;
    }
    pvVar4 = SpatialTree_CloneToLevel(pvVar4,param_1);
    AthenaList_Append((void *)((int)pvVar2 + 0x18),(int)pvVar4);
    iVar1 = *(int *)((int)this + iVar3 * 4 + 0x20);
    if (*(int *)((int)this + 0x1c) <= iVar1) break;
    pvVar4 = *(void **)(*(int *)((int)this + 0x424) + iVar1 * 4);
    *(int *)((int)this + iVar3 * 4 + 0x20) = iVar1 + 1;
  }
  ExceptionList = local_c;
  return pvVar2;
}

