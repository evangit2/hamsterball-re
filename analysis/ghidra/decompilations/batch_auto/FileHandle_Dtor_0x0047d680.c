
void __fastcall FileHandle_Dtor(undefined4 *param_1)

{
  *param_1 = &PTR_StreamReader_DeletingDtor_Close_004db2cc;
  __close(param_1[1]);
  if ((void *)param_1[2] != (void *)0x0) {
    _free((void *)param_1[2]);
    param_1[2] = 0;
  }
  return;
}

