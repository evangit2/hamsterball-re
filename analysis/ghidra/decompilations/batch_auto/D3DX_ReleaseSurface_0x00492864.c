
undefined4 __fastcall D3DX_ReleaseSurface(int *param_1)

{
  int *piVar1;
  
  piVar1 = (int *)*param_1;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x28))(piVar1);
  }
  *param_1 = 0;
  return 0;
}

