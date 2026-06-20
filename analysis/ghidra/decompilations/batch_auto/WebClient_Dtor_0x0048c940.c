
void __fastcall WebClient_Dtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  uint local_4;
  
  puStack_8 = &LAB_004ce096;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_WebClient_DeletingDtor_004dbe18;
  local_4 = 1;
  SoundDevice_ReleaseWindow((int)param_1);
  DAT_00535240 = DAT_00535240 + -1;
  if (DAT_00535240 == 0) {
    Ordinal_116();
  }
  AthenaString_dtor(param_1 + 0x10);
  local_4 = local_4 & 0xffffff00;
  AthenaString_dtor(param_1 + 9);
  local_4 = 0xffffffff;
  AthenaString_dtor(param_1 + 2);
  ExceptionList = pvStack_c;
  return;
}

