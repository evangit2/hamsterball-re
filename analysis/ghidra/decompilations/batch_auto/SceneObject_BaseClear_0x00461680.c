
void __fastcall SceneObject_BaseClear(int param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004ccfcb;
  local_c = ExceptionList;
  local_4 = 5;
  ExceptionList = &local_c;
  Vec3List_Free((undefined4 *)(param_1 + 0xcac));
  local_4._0_1_ = 4;
  Vec3List_Free((undefined4 *)(param_1 + 0x894));
  local_4._0_1_ = 3;
  Vec3List_Free((undefined4 *)(param_1 + 0x478));
  local_4._0_1_ = 2;
  Matrix_Identity((undefined4 *)(param_1 + 0x464));
  local_4._0_1_ = 1;
  Matrix_Identity((undefined4 *)(param_1 + 0x450));
  local_4 = (uint)local_4._1_3_ << 8;
  Vec3List_Free((undefined4 *)(param_1 + 0x1c));
  if (0xf < *(uint *)(param_1 + 0x18)) {
    _free(*(void **)(param_1 + 4));
  }
  *(undefined4 *)(param_1 + 0x18) = 0xf;
  *(undefined4 *)(param_1 + 0x14) = 0;
  *(undefined1 *)(param_1 + 4) = 0;
  ExceptionList = local_c;
  return;
}

