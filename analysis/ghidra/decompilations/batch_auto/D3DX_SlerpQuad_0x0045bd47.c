
undefined4
D3DX_SlerpQuad(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
              undefined4 param_5,float param_6)

{
  float fVar1;
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  
  D3DX_ShaderDispatch_noarg4(local_24,param_2,param_5,param_6);
  D3DX_ShaderDispatch_noarg4(local_14,param_3,param_4,param_6);
  fVar1 = (1.0 - param_6) * param_6;
  D3DX_ShaderDispatch_noarg4(param_1,local_24,local_14,fVar1 + fVar1);
  return param_1;
}

