
undefined4 __cdecl
UshortArray_PushBack(undefined4 *param_1,undefined2 *param_2,uint *param_3,uint *param_4)

{
  HANDLE hHeap;
  undefined2 *_Memory;
  undefined2 *puVar1;
  undefined4 uVar2;
  undefined2 *puVar3;
  DWORD dwFlags;
  LPCVOID lpMem;
  uint local_8;
  
  _Memory = (undefined2 *)*param_1;
  if (_Memory == (undefined2 *)0x0) {
    lpMem = (LPCVOID)0x0;
    dwFlags = 0;
    hHeap = GetProcessHeap();
    HeapValidate(hHeap,dwFlags,lpMem);
    _Memory = operator_new(4);
    if (_Memory != (undefined2 *)0x0) {
      *param_3 = 0;
      *param_4 = 2;
      goto LAB_0047e894;
    }
LAB_0047e8ab:
    uVar2 = 0;
  }
  else {
LAB_0047e894:
    puVar1 = _Memory;
    if (*param_3 == *param_4) {
      puVar1 = operator_new(*param_4 << 2);
      if (puVar1 == (undefined2 *)0x0) goto LAB_0047e8ab;
      local_8 = 0;
      if (*param_3 != 0) {
        puVar3 = puVar1;
        do {
          local_8 = local_8 + 1;
          *puVar3 = *(undefined2 *)(((int)_Memory - (int)puVar1) + (int)puVar3);
          puVar3 = puVar3 + 1;
        } while (local_8 < *param_3);
      }
      *param_4 = *param_4 << 1;
      _free(_Memory);
    }
    puVar1[*param_3] = *param_2;
    *param_3 = *param_3 + 1;
    *param_1 = puVar1;
    uVar2 = 1;
  }
  return uVar2;
}

