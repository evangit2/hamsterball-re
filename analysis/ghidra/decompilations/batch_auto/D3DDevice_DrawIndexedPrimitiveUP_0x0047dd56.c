
void D3DDevice_DrawIndexedPrimitiveUP
               (int *param_1,undefined4 param_2,uint *param_3,undefined4 param_4,undefined4 param_5)

{
  int iVar1;
  uint *puVar2;
  
  if (param_3 == (uint *)0x0) {
    puVar2 = (uint *)param_1[1];
  }
  else {
    iVar1 = D3DX_ParseDeclarationType(param_3,(uint *)&param_3);
    puVar2 = param_3;
    if (iVar1 < 0) {
      return;
    }
  }
  (**(code **)(*param_1 + 0x28))(param_1,param_2,puVar2,param_4,param_5);
  return;
}

