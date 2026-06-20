
void __fastcall Exception_dtor(exception *param_1)

{
  *(undefined ***)param_1 = &PTR_Exception_DeletingDtor_004e998c;
  if (0xf < *(uint *)(param_1 + 0x24)) {
    _free(*(void **)(param_1 + 0x10));
  }
  *(undefined4 *)(param_1 + 0x24) = 0xf;
  *(undefined4 *)(param_1 + 0x20) = 0;
  param_1[0x10] = (exception)0x0;
  exception::~exception(param_1);
  return;
}

