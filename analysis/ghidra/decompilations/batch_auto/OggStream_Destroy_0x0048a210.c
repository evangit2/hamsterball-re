
undefined4 __cdecl OggStream_Destroy(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  if (param_1 != (undefined4 *)0x0) {
    Pool_FreeList((int *)param_1[2]);
    puVar2 = (undefined4 *)*param_1;
    param_1[2] = 0;
    param_1[1] = 0;
    param_1[3] = 0;
    param_1[4] = 0;
    param_1[5] = 0;
    param_1[6] = 0;
    puVar2[3] = 1;
    AthenaList_FreeAllChunks(puVar2);
    puVar2 = param_1;
    for (iVar1 = 7; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    _free(param_1);
  }
  return 0;
}

