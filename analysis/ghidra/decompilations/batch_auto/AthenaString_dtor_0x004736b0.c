
void __fastcall AthenaString_dtor(undefined4 *param_1)

{
  *param_1 = &PTR_AthenaCString_DeletingDtor_004d290c;
  _free((void *)param_1[1]);
  param_1[1] = 0;
  param_1[5] = 0;
  param_1[2] = 0;
  return;
}

