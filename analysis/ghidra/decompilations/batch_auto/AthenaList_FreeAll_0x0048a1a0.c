
undefined4 __cdecl AthenaList_FreeAll(undefined4 *param_1)

{
  if (param_1 != (undefined4 *)0x0) {
    Pool_FreeList((int *)*param_1);
    Pool_FreeList((int *)param_1[2]);
    *param_1 = 0;
    param_1[1] = 0;
    param_1[2] = 0;
    param_1[3] = 0;
  }
  return 0;
}

