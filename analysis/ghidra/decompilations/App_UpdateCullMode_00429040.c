// App_UpdateCullMode @ 0x00429040
// Decompiled via GhidraMCP v5.12.0-headless
// Session 2928 - Batch decompilation

void __fastcall App_UpdateCullMode(int param_1)

{
  int iVar1;
  
  NoOp();
  iVar1 = *(int *)(param_1 + 0x174);
  *(undefined1 *)(iVar1 + 0x7d2) = *(undefined1 *)(param_1 + 0x236);
  Gfx_SetCullMode(iVar1);
  return;
}