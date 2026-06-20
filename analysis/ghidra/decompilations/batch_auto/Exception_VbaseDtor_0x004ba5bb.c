
void __fastcall Exception_VbaseDtor(exception *param_1)

{
  *(undefined ***)param_1 = &PTR_Exception_ScalarDeletingDtor_004e99a4;
  Exception_dtor(param_1);
  return;
}

