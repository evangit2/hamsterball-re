
int __fastcall SceneObject_BaseInit(int param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004ccfcb;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined4 *)(param_1 + 0x18) = 0xf;
  *(undefined4 *)(param_1 + 0x14) = 0;
  *(undefined1 *)(param_1 + 4) = 0;
  local_4 = 0;
  AthenaList_Init((void *)(param_1 + 0x1c),0);
  local_4._0_1_ = 1;
  Vec3_Init((undefined4 *)(param_1 + 0x450));
  local_4._0_1_ = 2;
  Vec3_Init((undefined4 *)(param_1 + 0x464));
  local_4._0_1_ = 3;
  AthenaList_Init((void *)(param_1 + 0x478),0);
  local_4._0_1_ = 4;
  AthenaList_Init((void *)(param_1 + 0x894),0);
  local_4 = CONCAT31(local_4._1_3_,5);
  AthenaList_Init((void *)(param_1 + 0xcac),0);
  ExceptionList = local_c;
  return param_1;
}

