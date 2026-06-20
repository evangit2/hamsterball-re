
void __cdecl Vorbis_FreeDecoder(undefined4 *param_1)

{
  void *_Memory;
  void *_Memory_00;
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  if (param_1 != (undefined4 *)0x0) {
    iVar4 = param_1[1];
    if (iVar4 == 0) {
      iVar2 = 0;
    }
    else {
      iVar2 = *(int *)(iVar4 + 0x1c);
    }
    _Memory = (void *)param_1[0x12];
    if (param_1[2] != 0) {
      iVar3 = 0;
      if (0 < *(int *)(iVar4 + 4)) {
        do {
          _Memory_00 = *(void **)(param_1[2] + iVar3 * 4);
          if (_Memory_00 != (void *)0x0) {
            _free(_Memory_00);
          }
          iVar3 = iVar3 + 1;
        } while (iVar3 < *(int *)(iVar4 + 4));
      }
      _free((void *)param_1[2]);
      if ((void *)param_1[3] != (void *)0x0) {
        _free((void *)param_1[3]);
      }
    }
    if ((iVar2 != 0) && (iVar4 = 0, 0 < *(int *)(iVar2 + 8))) {
      piVar1 = (int *)(iVar2 + 0x20);
      do {
        if ((_Memory != (void *)0x0) && (*(int *)((int)_Memory + 0xc) != 0)) {
          (**(code **)((&PTR_PTR_004fa7d8)[*(int *)(iVar2 + 0x120 + *(int *)(*piVar1 + 0xc) * 4)] +
                      0xc))(*(undefined4 *)(*(int *)((int)_Memory + 0xc) + iVar4 * 4));
        }
        iVar4 = iVar4 + 1;
        piVar1 = piVar1 + 1;
      } while (iVar4 < *(int *)(iVar2 + 8));
    }
    if (_Memory != (void *)0x0) {
      if (*(void **)((int)_Memory + 0xc) != (void *)0x0) {
        _free(*(void **)((int)_Memory + 0xc));
      }
      _free(_Memory);
    }
    for (iVar4 = 0x14; iVar4 != 0; iVar4 = iVar4 + -1) {
      *param_1 = 0;
      param_1 = param_1 + 1;
    }
  }
  return;
}

