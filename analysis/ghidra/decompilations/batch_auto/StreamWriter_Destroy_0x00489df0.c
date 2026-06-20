
undefined4 __cdecl StreamWriter_Destroy(undefined4 *param_1)

{
  int iVar1;
  
  if (param_1 != (undefined4 *)0x0) {
    Pool_FreeList((int *)param_1[1]);
    Pool_FreeList((int *)param_1[3]);
    for (iVar1 = 0x14; iVar1 != 0; iVar1 = iVar1 + -1) {
      *param_1 = 0;
      param_1 = param_1 + 1;
    }
  }
  return 0;
}

