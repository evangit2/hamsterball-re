
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void Gfx_ScaleZ(float param_1)

{
  undefined1 local_40 [64];
  
  D3DX_ShaderDispatch_2c(local_40,param_1 * _DAT_004d8e58);
  Graphics_SetRenderState();
  return;
}

