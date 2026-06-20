
void __cdecl D3DResourcePool_Release(undefined4 *param_1)

{
  void *_Memory;
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  
  _Memory = (void *)param_1[7];
  if (_Memory != (void *)0x0) {
    iVar1 = 0;
    if (0 < *(int *)((int)_Memory + 8)) {
      puVar2 = (undefined4 *)((int)_Memory + 0x20);
      do {
        if ((void *)*puVar2 != (void *)0x0) {
          _free((void *)*puVar2);
        }
        iVar1 = iVar1 + 1;
        puVar2 = puVar2 + 1;
      } while (iVar1 < *(int *)((int)_Memory + 8));
    }
    iVar1 = 0;
    if (0 < *(int *)((int)_Memory + 0xc)) {
      puVar2 = (undefined4 *)((int)_Memory + 0x220);
      do {
        (**(code **)((&PTR_PTR_004fa7d8)[puVar2[-0x40]] + 8))(*puVar2);
        iVar1 = iVar1 + 1;
        puVar2 = puVar2 + 1;
      } while (iVar1 < *(int *)((int)_Memory + 0xc));
    }
    iVar1 = 0;
    if (0 < *(int *)((int)_Memory + 0x14)) {
      puVar2 = (undefined4 *)((int)_Memory + 0x520);
      do {
        (**(code **)((&PTR_PTR_004fa7c4)[puVar2[-0x40]] + 8))(*puVar2);
        iVar1 = iVar1 + 1;
        puVar2 = puVar2 + 1;
      } while (iVar1 < *(int *)((int)_Memory + 0x14));
    }
    iVar1 = 0;
    if (0 < *(int *)((int)_Memory + 0x18)) {
      puVar2 = (undefined4 *)((int)_Memory + 0x720);
      do {
        (**(code **)((&PTR_PTR_004fa7cc)[puVar2[-0x40]] + 8))(*puVar2);
        iVar1 = iVar1 + 1;
        puVar2 = puVar2 + 1;
      } while (iVar1 < *(int *)((int)_Memory + 0x18));
    }
    iVar1 = 0;
    if (0 < *(int *)((int)_Memory + 0x1c)) {
      iVar3 = 0;
      puVar2 = (undefined4 *)((int)_Memory + 0x820);
      do {
        if ((undefined4 *)*puVar2 != (undefined4 *)0x0) {
          Huffman_FreeTable((undefined4 *)*puVar2);
        }
        if (*(int *)((int)_Memory + 0xc20) != 0) {
          Huffman_FreeTableExtended((undefined4 *)(iVar3 + *(int *)((int)_Memory + 0xc20)));
        }
        iVar1 = iVar1 + 1;
        puVar2 = puVar2 + 1;
        iVar3 = iVar3 + 0x34;
      } while (iVar1 < *(int *)((int)_Memory + 0x1c));
    }
    if (*(void **)((int)_Memory + 0xc20) != (void *)0x0) {
      _free(*(void **)((int)_Memory + 0xc20));
    }
    _free(_Memory);
  }
  for (iVar1 = 8; iVar1 != 0; iVar1 = iVar1 + -1) {
    *param_1 = 0;
    param_1 = param_1 + 1;
  }
  return;
}

