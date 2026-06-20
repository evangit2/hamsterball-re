
void __fastcall D3DX_BoxFilter2x_Dispatch(int *param_1)

{
  int iVar1;
  
  iVar1 = *param_1;
  (*(code *)PTR_D3DX_BoxFilter2x_Init_004fa638)
            (*(undefined4 *)(param_1[1] + 0x18),*(undefined4 *)(iVar1 + 0x18),
             *(undefined4 *)(iVar1 + 0x1058),*(undefined4 *)(iVar1 + 0x105c),
             *(undefined4 *)(iVar1 + 0x1050),*(undefined4 *)(param_1[1] + 0x1050));
  return;
}

