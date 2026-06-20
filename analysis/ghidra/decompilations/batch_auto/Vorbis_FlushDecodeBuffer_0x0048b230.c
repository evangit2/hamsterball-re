
void __cdecl Vorbis_FlushDecodeBuffer(int param_1)

{
  undefined4 *puVar1;
  undefined4 *_Memory;
  void *pvVar2;
  
  _Memory = *(undefined4 **)(param_1 + 0x5c);
  while (_Memory != (undefined4 *)0x0) {
    puVar1 = (undefined4 *)_Memory[1];
    _free((void *)*_Memory);
    *_Memory = 0;
    _Memory[1] = 0;
    _free(_Memory);
    _Memory = puVar1;
  }
  if (*(int *)(param_1 + 0x58) != 0) {
    pvVar2 = _realloc(*(void **)(param_1 + 0x4c),*(int *)(param_1 + 0x54) + *(int *)(param_1 + 0x58)
                     );
    *(void **)(param_1 + 0x4c) = pvVar2;
    *(int *)(param_1 + 0x54) = *(int *)(param_1 + 0x54) + *(int *)(param_1 + 0x58);
    *(undefined4 *)(param_1 + 0x58) = 0;
  }
  *(undefined4 *)(param_1 + 0x50) = 0;
  *(undefined4 *)(param_1 + 0x5c) = 0;
  return;
}

