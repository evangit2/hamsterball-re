
undefined4 __cdecl DynArray_Grow(int *param_1,undefined4 *param_2,uint *param_3,uint *param_4)

{
  HANDLE hHeap;
  undefined4 *_Memory;
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  DWORD dwFlags;
  LPCVOID lpMem;
  uint local_8;
  
  _Memory = (undefined4 *)*param_1;
  if (_Memory == (undefined4 *)0x0) {
    lpMem = (LPCVOID)0x0;
    dwFlags = 0;
    hHeap = GetProcessHeap();
    HeapValidate(hHeap,dwFlags,lpMem);
    _Memory = operator_new(8);
    if (_Memory != (undefined4 *)0x0) {
      *param_3 = 0;
      *param_4 = 2;
      goto LAB_0047eb17;
    }
LAB_0047eb2e:
    uVar2 = 0;
  }
  else {
LAB_0047eb17:
    puVar1 = _Memory;
    if (*param_3 == *param_4) {
      puVar1 = operator_new(*param_4 << 3);
      if (puVar1 == (undefined4 *)0x0) goto LAB_0047eb2e;
      local_8 = 0;
      if (*param_3 != 0) {
        puVar3 = puVar1;
        do {
          local_8 = local_8 + 1;
          *puVar3 = *(undefined4 *)(((int)_Memory - (int)puVar1) + (int)puVar3);
          puVar3 = puVar3 + 1;
        } while (local_8 < *param_3);
      }
      *param_4 = *param_4 << 1;
      _free(_Memory);
    }
    puVar1[*param_3] = *param_2;
    *param_3 = *param_3 + 1;
    *param_1 = (int)puVar1;
    uVar2 = 1;
  }
  return uVar2;
}

