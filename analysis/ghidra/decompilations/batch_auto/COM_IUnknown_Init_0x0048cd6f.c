
void __fastcall COM_IUnknown_Init(undefined4 *param_1)

{
  param_1[3] = 0;
  param_1[2] = 0;
  *param_1 = &PTR_COM_QueryInterface_004dbeac;
  param_1[1] = 1;
  return;
}

