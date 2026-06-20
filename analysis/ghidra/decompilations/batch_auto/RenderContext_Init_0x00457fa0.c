
undefined4 * __fastcall RenderContext_Init(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  *param_1 = &PTR_LAB_004d8e68;
  param_1[0x12] = 0;
  puVar2 = param_1;
  for (iVar1 = 0x11; puVar2 = puVar2 + 1, iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
  }
  *(undefined1 *)(param_1 + 0x13) = 0;
  *(undefined1 *)((int)param_1 + 0x4d) = 0;
  return param_1;
}

