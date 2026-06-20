
int D3DResource_ReleaseA(undefined4 *param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  
  piVar1 = (int *)param_1[0x19];
  if (piVar1 == (int *)0x0) {
    iVar2 = param_1[0x18];
    iVar3 = iVar2 + -1;
    param_1[0x18] = iVar3;
    if (iVar2 == 1) {
      D3DDevice_ReleaseResourcesA(param_1);
      _free(param_1);
    }
  }
  else {
    iVar3 = (**(code **)(*piVar1 + 8))(piVar1);
  }
  return iVar3;
}

