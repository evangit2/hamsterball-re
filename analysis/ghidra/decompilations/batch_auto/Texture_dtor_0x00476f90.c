
void __fastcall Texture_dtor(undefined4 *param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  int iVar3;
  
  iVar3 = 0;
  *param_1 = &PTR_Texture_DeletingDtor_004da648;
  if (0 < (int)param_1[8]) {
    puVar2 = param_1 + 9;
    do {
      (**(code **)(*(int *)*puVar2 + 8))((int *)*puVar2);
      iVar3 = iVar3 + 1;
      puVar2 = puVar2 + 1;
    } while (iVar3 < (int)param_1[8]);
  }
  piVar1 = (int *)param_1[1];
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[1] = 0;
  }
  if ((void *)param_1[2] != (void *)0x0) {
    _free((void *)param_1[2]);
    param_1[2] = 0;
  }
  return;
}

