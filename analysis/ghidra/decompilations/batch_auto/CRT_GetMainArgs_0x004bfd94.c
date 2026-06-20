
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall CRT_GetMainArgs(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int local_8;
  
  local_8 = param_1;
  if (DAT_00536aec == 0) {
    ___initmbctable();
  }
  DAT_00535434 = 0;
  GetModuleFileNameA((HMODULE)0x0,&DAT_00535330,0x104);
  _DAT_00535300 = &DAT_00535330;
  CRT_ParseCommandLine((void *)0x0,(undefined4 *)0x0,&local_8);
  iVar1 = local_8;
  puVar2 = _malloc(param_1 + local_8 * 4);
  if (puVar2 == (undefined4 *)0x0) {
    uVar3 = 0xffffffff;
  }
  else {
    CRT_ParseCommandLine(puVar2 + iVar1,puVar2,&local_8);
    _DAT_005352e4 = local_8 + -1;
    uVar3 = 0;
    _DAT_005352e8 = puVar2;
  }
  return uVar3;
}

