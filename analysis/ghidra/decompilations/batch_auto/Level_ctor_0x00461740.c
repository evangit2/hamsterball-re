
void * __thiscall Level_ctor(void *this,undefined4 param_1)

{
  void *pvVar1;
  int iVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ccf76;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  SpriteAnim_Ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_Level_DeletingDtor_004d8fb0;
  AthenaList_Init((void *)((int)this + 0x18),0);
  local_4._0_1_ = 1;
  Timer_Init((undefined4 *)((int)this + 0x438));
  local_4._0_1_ = 2;
  AthenaList_Init((void *)((int)this + 0x488),0);
  local_4._0_1_ = 3;
  AthenaList_Init((void *)((int)this + 0x8a0),0);
  local_4._0_1_ = 4;
  AthenaList_Init((void *)((int)this + 0xcb8),0);
  local_4._0_1_ = 5;
  AthenaList_Free((int)this + 0x18);
  *(undefined1 *)((int)this + 0x430) = 0;
  *(undefined4 *)((int)this + 0x434) = 0;
  *(undefined1 *)((int)this + 0x431) = 0;
  *(undefined1 *)((int)this + 0x484) = 0;
  pvVar1 = operator_new(0x10d4);
  local_4 = CONCAT31(local_4._1_3_,6);
  if (pvVar1 == (void *)0x0) {
    iVar2 = 0;
  }
  else {
    iVar2 = SceneObject_BaseInit((int)pvVar1);
  }
  *(int *)((int)this + 0x480) = iVar2;
  *(undefined1 *)(iVar2 + 0x890) = 0;
  *(undefined4 *)(*(int *)((int)this + 0x480) + 0x444) = 0;
  *(undefined4 *)(*(int *)((int)this + 0x480) + 0x448) = 0;
  *(undefined1 *)(*(int *)((int)this + 0x480) + 0x434) = 0;
  *(undefined4 *)(*(int *)((int)this + 0x480) + 0x440) = 0;
  *(undefined1 *)(*(int *)((int)this + 0x480) + 0x10c4) = 0;
  *(undefined1 *)(*(int *)((int)this + 0x480) + 0x10d0) = 0;
  *(undefined4 *)(*(int *)((int)this + 0x480) + 0x44c) = 0;
  *(undefined4 *)(*(int *)((int)this + 0x480) + 0x10c8) = 0;
  ExceptionList = local_c;
  return this;
}

