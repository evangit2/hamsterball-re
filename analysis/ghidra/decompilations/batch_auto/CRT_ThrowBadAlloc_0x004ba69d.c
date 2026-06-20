
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void CRT_ThrowBadAlloc(void)

{
  undefined **local_14 [3];
  char *local_8;
  
  if ((_DAT_005352b4 & 1) == 0) {
    _DAT_005352b4 = _DAT_005352b4 | 1;
    local_8 = "bad allocation";
    exception::exception((exception *)&DAT_005352a8,&local_8);
    _DAT_005352a8 = &PTR_BadAlloc_ScalarDeletingDtor_004e99d8;
    _atexit(FUN_004ce580);
  }
  exception::exception((exception *)local_14,(exception *)&DAT_005352a8);
  local_14[0] = &PTR_BadAlloc_ScalarDeletingDtor_004e99d8;
                    /* WARNING: Subroutine does not return */
  __CxxThrowException_8(local_14,&DAT_004f5b70);
}

