
void __fastcall App_ResetFrame(int param_1)

{
  Gfx_ResetRenderState(*(int *)(param_1 + 0x174));
  Graphics_ClearViewport
            (*(void **)(param_1 + 0x174),*(undefined4 *)((int)*(void **)(param_1 + 0x174) + 0x738));
  return;
}

