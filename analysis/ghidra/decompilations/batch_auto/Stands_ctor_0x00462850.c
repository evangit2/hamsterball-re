
void * __thiscall Stands_ctor(void *this,int param_1)

{
  void *this_00;
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  void *pvVar5;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ccf76;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  SpriteAnim_Ctor(this,*(undefined4 *)(param_1 + 4));
  this_00 = (void *)((int)this + 0x18);
  local_4 = 0;
  *(undefined ***)this = &PTR_Level_DeletingDtor_004d8fb0;
  AthenaList_Init(this_00,0);
  local_4._0_1_ = 1;
  Timer_Init((undefined4 *)((int)this + 0x438));
  local_4._0_1_ = 2;
  AthenaList_Init((void *)((int)this + 0x488),0);
  local_4._0_1_ = 3;
  AthenaList_Init((void *)((int)this + 0x8a0),0);
  local_4._0_1_ = 4;
  AthenaList_Init((void *)((int)this + 0xcb8),0);
  local_4._0_1_ = 5;
  AthenaList_Free((int)this_00);
  *(undefined1 *)((int)this + 0x430) = *(undefined1 *)(param_1 + 0x430);
  *(undefined1 *)((int)this + 0x431) = 1;
  puVar2 = operator_new(0x44);
  local_4._0_1_ = 6;
  if (puVar2 == (undefined4 *)0x0) {
    uVar3 = 0;
  }
  else {
    uVar3 = Timer_Init(puVar2);
  }
  *(undefined4 *)((int)this + 0x434) = uVar3;
  *(undefined1 *)((int)this + 0x484) = 1;
  *(undefined4 *)((int)this + 0x480) = *(undefined4 *)(param_1 + 0x480);
  local_4 = CONCAT31(local_4._1_3_,5);
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
    pvVar5 = SpatialTree_CloneToLevel(pvVar5,(int)this);
    AthenaList_Append(this_00,(int)pvVar5);
    iVar1 = *(int *)(param_1 + 0x20 + iVar4 * 4);
    if (*(int *)(param_1 + 0x1c) <= iVar1) break;
    pvVar5 = *(void **)(*(int *)(param_1 + 0x424) + iVar1 * 4);
    *(int *)(param_1 + 0x20 + iVar4 * 4) = iVar1 + 1;
  }
  ExceptionList = local_c;
  return this;
}

