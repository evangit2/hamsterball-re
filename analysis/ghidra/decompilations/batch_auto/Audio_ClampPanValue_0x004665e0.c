
int Audio_ClampPanValue(void)

{
  int iVar1;
  ulonglong uVar2;
  
  uVar2 = __ftol2();
  iVar1 = (int)uVar2;
  if (iVar1 < -2000) {
    iVar1 = -10000;
  }
  return iVar1;
}

