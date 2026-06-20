
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void CRT_InitFPState(void)

{
  CRT_InitFPFuncTable();
  _DAT_005352bc = __ms_p5_mp_test_fdiv();
  CRT_SetFPUAffinity();
  return;
}

