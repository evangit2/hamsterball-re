
undefined4 * __thiscall Level_CloneTree(void *this,int param_1)

{
  int iVar1;
  undefined4 *this_00;
  int iVar2;
  undefined4 *puVar3;
  void *pvVar4;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd25b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  this_00 = operator_new(0x10d0);
  local_4 = 0;
  if (this_00 == (undefined4 *)0x0) {
    this_00 = (undefined4 *)0x0;
  }
  else {
    Level_ctor(this_00,*(undefined4 *)((int)this + 4));
    *this_00 = &PTR_Level_DeletingDtor2_004d9068;
  }
  pvVar4 = (void *)this_00[0x120];
  local_4 = 0xffffffff;
  if (pvVar4 != (void *)0x0) {
    SceneObject_BaseClear((int)pvVar4);
    _free(pvVar4);
  }
  *(undefined1 *)(this_00 + 0x121) = 1;
  this_00[0x120] = *(undefined4 *)((int)this + 0x480);
  this_00[2] = *(undefined4 *)((int)this + 8);
  *(undefined1 *)((int)this_00 + 0xd) = 0;
  *(undefined1 *)(this_00 + 0x10c) = *(undefined1 *)((int)this + 0x430);
  this_00[0x11f] = param_1;
  this_00[0x10d] = *(undefined4 *)(param_1 + 0x434);
  *(undefined1 *)((int)this_00 + 0x431) = 0;
  iVar2 = AthenaList_NextIndex((int)this + 0x18);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x20) = 0;
  if (*(int *)((int)this + 0x1c) < 1) {
    pvVar4 = (void *)0x0;
  }
  else {
    pvVar4 = (void *)**(undefined4 **)((int)this + 0x424);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x20) = 1;
  }
  while( true ) {
    if (pvVar4 == (void *)0x0) {
      ExceptionList = local_c;
      return this_00;
    }
    puVar3 = Level_CloneTree(pvVar4,param_1);
    AthenaList_Append(this_00 + 6,(int)puVar3);
    iVar1 = *(int *)((int)this + iVar2 * 4 + 0x20);
    if (*(int *)((int)this + 0x1c) <= iVar1) break;
    pvVar4 = *(void **)(*(int *)((int)this + 0x424) + iVar1 * 4);
    *(int *)((int)this + iVar2 * 4 + 0x20) = iVar1 + 1;
  }
  ExceptionList = local_c;
  return this_00;
}

