
void __cdecl AthenaList_FreeAllChunks(undefined4 *param_1)

{
  undefined4 *_Memory;
  void *_Memory_00;
  undefined4 *puVar1;
  void *pvVar2;
  
  if (param_1[3] != 0) {
    _Memory = (undefined4 *)*param_1;
    _Memory_00 = (void *)param_1[1];
    if (param_1[2] == 0) {
      _free(param_1);
      return;
    }
    while (_Memory != (undefined4 *)0x0) {
      puVar1 = (undefined4 *)_Memory[3];
      if ((void *)*_Memory != (void *)0x0) {
        _free((void *)*_Memory);
      }
      _free(_Memory);
      _Memory = puVar1;
    }
    *param_1 = 0;
    while (_Memory_00 != (void *)0x0) {
      pvVar2 = *(void **)((int)_Memory_00 + 0xc);
      _free(_Memory_00);
      _Memory_00 = pvVar2;
    }
    param_1[1] = 0;
  }
  return;
}

