
int CRT_Initterm(void)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (PTR_CRT_InitFPState_004fc458 != (undefined *)0x0) {
    (*(code *)PTR_CRT_InitFPState_004fc458)();
  }
  iVar1 = 0;
  puVar2 = &DAT_004f702c;
  do {
    if (iVar1 != 0) {
      return iVar1;
    }
    if ((code *)*puVar2 != (code *)0x0) {
      iVar1 = (*(code *)*puVar2)();
    }
    puVar2 = puVar2 + 1;
  } while (puVar2 < &DAT_004f7044);
  if (iVar1 == 0) {
    _atexit(FUN_004c019a);
    puVar2 = &DAT_004f7000;
    do {
      if ((code *)*puVar2 != (code *)0x0) {
        (*(code *)*puVar2)();
      }
      puVar2 = puVar2 + 1;
    } while (puVar2 < &DAT_004f7028);
    iVar1 = 0;
  }
  return iVar1;
}

