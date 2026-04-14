
undefined4 * __thiscall FUN_004027f0(void *this,byte param_1)

{
  Ball_Cleanup(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}
