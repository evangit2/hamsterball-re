
undefined4 __cdecl WideArray_Grow(undefined4 *param_1,uint param_2,int param_3,uint *param_4)

{
  HANDLE hHeap;
  undefined2 *puVar1;
  undefined2 *puVar2;
  undefined2 *_Memory;
  int local_8;
  
  _Memory = (undefined2 *)*param_1;
  if (_Memory == (undefined2 *)0x0) {
    puVar1 = _Memory;
    hHeap = GetProcessHeap();
    HeapValidate(hHeap,(DWORD)_Memory,puVar1);
    puVar1 = operator_new(param_2 * 2);
    *param_4 = param_2;
  }
  else {
    puVar1 = _Memory;
    if (*param_4 < param_2) {
      puVar1 = operator_new(param_2 * 2);
      if (puVar1 == (undefined2 *)0x0) {
        return 0;
      }
      if (param_3 != 0) {
        local_8 = param_3;
        puVar2 = puVar1;
        do {
          *puVar2 = *(undefined2 *)(((int)_Memory - (int)puVar1) + (int)puVar2);
          puVar2 = puVar2 + 1;
          local_8 = local_8 + -1;
        } while (local_8 != 0);
      }
      *param_4 = param_2;
      _free(_Memory);
    }
  }
  *param_1 = puVar1;
  return 1;
}

