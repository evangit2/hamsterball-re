
void __fastcall StreamReader_dtor(undefined4 *param_1)

{
  *param_1 = &PTR_FileStream_DeletingDtor_004d91bc;
  if (param_1[4] != 0) {
    __close(param_1[4]);
  }
  _free((void *)param_1[2]);
  param_1[2] = 0;
  return;
}

