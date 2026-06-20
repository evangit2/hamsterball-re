
void __fastcall SceneObject_Cleanup(int *param_1)

{
  int iVar1;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004cd57a;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = (int)&PTR_SceneObject_dtor_004d934c;
  local_4 = 2;
  iVar1 = 0x710;
  do {
    if (*(int **)(iVar1 + param_1[1]) == param_1) {
      (**(code **)(*param_1 + 0x10))(0);
      *(undefined4 *)(iVar1 + param_1[1]) = 0;
    }
    iVar1 = iVar1 + 4;
  } while (iVar1 < 0x730);
  local_4._0_1_ = 1;
  Matrix_Identity(param_1 + 0x2e);
  local_4 = (uint)local_4._1_3_ << 8;
  Matrix_Identity(param_1 + 0x29);
  local_4 = 0xffffffff;
  Matrix_Identity(param_1 + 0x24);
  ExceptionList = local_c;
  return;
}

