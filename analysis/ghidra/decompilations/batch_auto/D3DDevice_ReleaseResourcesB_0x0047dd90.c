
void __fastcall D3DDevice_ReleaseResourcesB(undefined4 *param_1)

{
  int *piVar1;
  
  piVar1 = (int *)param_1[8];
  *param_1 = &PTR_D3DDevice_Reset_004db3d0;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[8] = 0;
  }
  piVar1 = (int *)param_1[9];
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[9] = 0;
  }
  piVar1 = (int *)param_1[10];
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[10] = 0;
  }
  piVar1 = (int *)param_1[0xf];
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[0xf] = 0;
  }
  _free((void *)param_1[0x11]);
  _free((void *)param_1[0x12]);
  _free((void *)param_1[0x13]);
  _free((void *)param_1[0x14]);
  _free((void *)param_1[0xe]);
  return;
}

