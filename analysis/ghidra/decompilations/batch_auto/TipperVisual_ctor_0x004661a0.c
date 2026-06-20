
void * __thiscall TipperVisual_ctor(void *this,int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  void *pvVar5;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd283;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Level_ctor(this,*(undefined4 *)(param_1 + 4));
  local_4 = 0;
  *(undefined ***)this = &PTR_Level_DeletingDtor2_004d9068;
  AthenaList_Free((int)this + 0x18);
  pvVar5 = *(void **)((int)this + 0x480);
  if (pvVar5 != (void *)0x0) {
    SceneObject_BaseClear((int)pvVar5);
    _free(pvVar5);
  }
  *(undefined1 *)((int)this + 0x430) = *(undefined1 *)(param_1 + 0x430);
  *(undefined1 *)((int)this + 0x431) = 1;
  puVar2 = operator_new(0x44);
  local_4._0_1_ = 1;
  if (puVar2 == (undefined4 *)0x0) {
    uVar3 = 0;
  }
  else {
    uVar3 = Timer_Init(puVar2);
  }
  *(undefined4 *)((int)this + 0x434) = uVar3;
  *(undefined1 *)((int)this + 0x484) = 1;
  *(undefined4 *)((int)this + 0x480) = *(undefined4 *)(param_1 + 0x480);
  local_4 = (uint)local_4._1_3_ << 8;
  *(undefined4 *)((int)this + 8) = *(undefined4 *)(param_1 + 8);
  *(undefined1 *)((int)this + 0xd) = 0;
  *(void **)((int)this + 0x47c) = this;
  iVar4 = AthenaList_NextIndex(param_1 + 0x18);
  *(undefined4 *)(param_1 + 0x20 + iVar4 * 4) = 0;
  if (*(int *)(param_1 + 0x1c) < 1) {
    pvVar5 = (void *)0x0;
  }
  else {
    pvVar5 = (void *)**(undefined4 **)(param_1 + 0x424);
    *(undefined4 *)(param_1 + 0x20 + iVar4 * 4) = 1;
  }
  while( true ) {
    if (pvVar5 == (void *)0x0) {
      ExceptionList = local_c;
      return this;
    }
    puVar2 = Level_CloneTree(pvVar5,(int)this);
    AthenaList_Append((void *)((int)this + 0x18),(int)puVar2);
    iVar1 = *(int *)(param_1 + 0x20 + iVar4 * 4);
    if (*(int *)(param_1 + 0x1c) <= iVar1) break;
    pvVar5 = *(void **)(*(int *)(param_1 + 0x424) + iVar1 * 4);
    *(int *)(param_1 + 0x20 + iVar4 * 4) = iVar1 + 1;
  }
  ExceptionList = local_c;
  return this;
}

