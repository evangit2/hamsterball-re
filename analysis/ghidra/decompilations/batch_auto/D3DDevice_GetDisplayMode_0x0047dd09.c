
undefined4 D3DDevice_GetDisplayMode(int param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  
  if (param_2 == (undefined4 *)0x0) {
    uVar1 = 0x8876086c;
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x24) + 4))(*(int **)(param_1 + 0x24));
    *param_2 = *(undefined4 *)(param_1 + 0x24);
    uVar1 = 0;
  }
  return uVar1;
}

