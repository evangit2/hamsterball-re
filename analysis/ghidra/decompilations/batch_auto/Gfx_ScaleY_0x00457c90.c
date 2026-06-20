
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void Gfx_ScaleY(float param_1)

{
  undefined1 local_40 [64];
  
  D3DX_ShaderDispatch_2a(local_40,param_1 * _DAT_004d8e58);
  Graphics_SetRenderState();
  return;
}

