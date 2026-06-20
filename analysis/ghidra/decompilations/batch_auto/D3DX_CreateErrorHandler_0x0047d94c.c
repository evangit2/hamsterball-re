
int D3DX_CreateErrorHandler(undefined4 param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  int *piVar2;
  int iVar3;
  
  puVar1 = operator_new(0x10);
  if (puVar1 == (undefined4 *)0x0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = (int *)COM_IUnknown_Init(puVar1);
  }
  if (piVar2 == (int *)0x0) {
    iVar3 = -0x7ff8fff2;
  }
  else {
    iVar3 = (**(code **)(*piVar2 + 0x18))(param_1);
    if (-1 < iVar3) {
      *param_2 = piVar2;
      piVar2 = (int *)0x0;
    }
  }
  if (piVar2 != (int *)0x0) {
    (**(code **)(*piVar2 + 0x14))(1);
  }
  return iVar3;
}

