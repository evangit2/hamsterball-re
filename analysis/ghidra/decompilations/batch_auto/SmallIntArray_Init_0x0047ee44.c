
undefined4 __fastcall SmallIntArray_Init(undefined4 *param_1)

{
  void *pvVar1;
  uint uVar2;
  
  if (param_1[4] == 0) {
    pvVar1 = operator_new(param_1[3] << 2);
    param_1[4] = pvVar1;
    if (pvVar1 == (void *)0x0) {
      return 0x8007000e;
    }
  }
  uVar2 = 0;
  if (param_1[3] != 0) {
    do {
      *(undefined4 *)(param_1[4] + uVar2 * 4) = 0xffffffff;
      uVar2 = uVar2 + 1;
    } while (uVar2 < (uint)param_1[3]);
  }
  param_1[2] = 0;
  param_1[1] = 0;
  *param_1 = 0;
  return 0;
}

