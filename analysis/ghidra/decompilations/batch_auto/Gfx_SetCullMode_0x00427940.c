
void __fastcall Gfx_SetCullMode(int param_1)

{
  int *piVar1;
  
  if (*(int *)(param_1 + 0x708) == 2) {
    piVar1 = *(int **)(param_1 + 0x154);
    if (*(char *)(param_1 + 0x7d2) == '\0') {
      (**(code **)(*piVar1 + 200))(piVar1,0x16,2);
    }
    else {
      (**(code **)(*piVar1 + 200))(piVar1,0x16,3);
    }
    *(undefined4 *)(param_1 + 0x708) = 2;
  }
  if (*(int *)(param_1 + 0x708) == 3) {
    piVar1 = *(int **)(param_1 + 0x154);
    if (*(char *)(param_1 + 0x7d2) == '\0') {
      (**(code **)(*piVar1 + 200))(piVar1,0x16,3);
    }
    else {
      (**(code **)(*piVar1 + 200))(piVar1,0x16,2);
    }
    *(undefined4 *)(param_1 + 0x708) = 3;
  }
  if (*(int *)(param_1 + 0x708) == 1) {
    (**(code **)(**(int **)(param_1 + 0x154) + 200))(*(int **)(param_1 + 0x154),0x16,1);
    *(undefined4 *)(param_1 + 0x708) = 1;
  }
  return;
}

