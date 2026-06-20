
void __fastcall KeyboardDevice_dtor(undefined4 *param_1)

{
  int *piVar1;
  
  piVar1 = (int *)param_1[2];
  *param_1 = &PTR_KeyboardDevice_ScalarDtor_004d9840;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[2] = 0;
  }
  RegKey_Open(*(int *)(*(int *)(param_1[1] + 4) + 0x54));
  RegKey_WriteDword(*(void **)(*(int *)(param_1[1] + 4) + 0x54),"KeyUp",param_1[0x143]);
  RegKey_WriteDword(*(void **)(*(int *)(param_1[1] + 4) + 0x54),"KeyDown",param_1[0x144]);
  RegKey_WriteDword(*(void **)(*(int *)(param_1[1] + 4) + 0x54),"KeyLeft",param_1[0x145]);
  RegKey_WriteDword(*(void **)(*(int *)(param_1[1] + 4) + 0x54),"KeyRight",param_1[0x146]);
  RegKey_WriteDword(*(void **)(*(int *)(param_1[1] + 4) + 0x54),"KeyAction1",param_1[0x147]);
  RegKey_WriteDword(*(void **)(*(int *)(param_1[1] + 4) + 0x54),"KeyAction2",param_1[0x148]);
  RegKey_Close(*(int *)(*(int *)(param_1[1] + 4) + 0x54));
  return;
}

