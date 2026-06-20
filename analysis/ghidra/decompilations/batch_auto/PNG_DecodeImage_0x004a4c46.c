
void __cdecl PNG_DecodeImage(int *param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  int *extraout_ECX;
  int *this;
  undefined4 local_8;
  
  this = param_1;
  local_8 = PNG_SetTransformFiller((int)param_1);
  iVar1 = param_1[0x2f];
  param_1[0x30] = iVar1;
  iVar2 = iVar1;
  puVar3 = param_2;
  if (0 < local_8) {
    do {
      for (; iVar2 != 0; iVar2 = iVar2 + -1) {
        PNG_DecodeRow(this,param_1,(byte *)*puVar3,(byte *)0x0);
        puVar3 = puVar3 + 1;
        this = extraout_ECX;
      }
      local_8 = local_8 + -1;
      iVar2 = iVar1;
      puVar3 = param_2;
    } while (local_8 != 0);
  }
  return;
}

