
void __fastcall ArenaBoard_CollSlices_dtor(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cab68;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  _eh_vector_destructor_iterator_(param_1 + 0x11f8,0x418,5,Vec3List_Free);
  local_4 = 0xffffffff;
  ArenaBoard_dtor(param_1);
  ExceptionList = local_c;
  return;
}

