
undefined4 * __thiscall Texture_RemoveRef(void *this,undefined4 *param_1)

{
  int iVar1;
  
  iVar1 = param_1[4];
  param_1[4] = iVar1 + -1;
  if (iVar1 + -1 < 1) {
    IndexList_RemoveItem((void *)((int)this + 0x2e4),(int)param_1);
    (**(code **)*param_1)(1);
    return (undefined4 *)0x0;
  }
  return param_1;
}

