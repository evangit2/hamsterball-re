
void __fastcall BadAlloc_VbaseDtor(exception *param_1)

{
  *(undefined ***)param_1 = &PTR_BadAlloc_ScalarDeletingDtor_004e99d8;
  exception::~exception(param_1);
  return;
}

