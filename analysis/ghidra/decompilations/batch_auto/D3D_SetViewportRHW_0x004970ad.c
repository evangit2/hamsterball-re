
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall D3D_SetViewportRHW(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = _DAT_004d039c;
  if ((*(int *)(param_1 + 4) != 0x32545844) && (*(int *)(param_1 + 4) != 0x33545844)) {
    uVar1 = _DAT_004cf304;
  }
  *(undefined4 *)(param_1 + 0x106c) = uVar1;
  *(float *)(param_1 + 0x1070) = 1.0 / *(float *)(param_1 + 0x106c);
  *(float *)(param_1 + 0x1c) =
       (float)(int)ROUND(*(float *)(param_1 + 0x1c) * _DAT_004d5c24 + _DAT_004cf3f0) * _DAT_004d5c50
  ;
  *(float *)(param_1 + 0x20) =
       (float)(int)ROUND(*(float *)(param_1 + 0x20) * _DAT_004dc064 + _DAT_004cf3f0) * _DAT_004dc08c
  ;
  *(float *)(param_1 + 0x24) =
       (float)(int)ROUND(*(float *)(param_1 + 0x24) * _DAT_004d5c24 + _DAT_004cf3f0) * _DAT_004d5c50
  ;
  *(float *)(param_1 + 0x28) =
       (float)(int)ROUND(*(float *)(param_1 + 0x106c) * *(float *)(param_1 + 0x28) + _DAT_004cf3f0)
       * *(float *)(param_1 + 0x1070);
  return;
}

