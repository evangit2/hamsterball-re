
void __fastcall FlagWaver_AllocBuffers(int param_1)

{
  int iVar1;
  int *piVar2;
  void *pvVar3;
  
  piVar2 = *(int **)(param_1 + 0x84);
  if (piVar2 != (int *)0x0) {
    (**(code **)(*piVar2 + 8))(piVar2);
  }
  *(undefined4 *)(param_1 + 0x84) = 0;
  _free(*(void **)(param_1 + 0xc));
  iVar1 = *(int *)(param_1 + 8) * 2 + 2;
  *(int *)(param_1 + 0x88) = iVar1;
  *(undefined4 *)(param_1 + 0xc) = 0;
  *(float *)(param_1 + 0x18) = *(float *)(param_1 + 0x10) / (float)*(int *)(param_1 + 8);
  pvVar3 = operator_new(iVar1 * 0x20);
  *(void **)(param_1 + 0xc) = pvVar3;
  piVar2 = *(int **)(*(int *)(param_1 + 4) + 0x154);
  (**(code **)(*piVar2 + 0x5c))
            (piVar2,*(int *)(param_1 + 0x88) << 5,0x200,0x112,1,(undefined4 *)(param_1 + 0x84));
  return;
}

