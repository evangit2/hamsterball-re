
void __fastcall SceneObject_dtor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cd41e;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_SceneObject_ScalarDtor_004d9170;
  local_4 = 2;
  Vec3List_Free(param_1 + 0x113);
  local_4 = CONCAT31(local_4._1_3_,1);
  Vec3List_Free(param_1 + 0xd);
  param_1[6] = &PTR_LAB_004cf584;
  *param_1 = &PTR_LAB_004cf584;
  ExceptionList = local_c;
  return;
}

