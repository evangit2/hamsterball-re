
undefined4 *
D3DX_SquadInterpolate
          (undefined4 *param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4,
          float param_5,float param_6)

{
  float fVar1;
  int iVar2;
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  
  fVar1 = param_5 + param_6;
  iVar2 = Float_IsBetween(fVar1,0.0);
  if (iVar2 == 0) {
    D3DX_ShaderDispatch_noarg4(local_24,param_2,param_3,fVar1);
    D3DX_ShaderDispatch_noarg4(local_14,param_2,param_4,fVar1);
    D3DX_ShaderDispatch_noarg4(param_1,local_24,local_14,param_6 / fVar1);
  }
  else if (param_1 != param_2) {
    *param_1 = *param_2;
    param_1[1] = param_2[1];
    param_1[2] = param_2[2];
    param_1[3] = param_2[3];
  }
  return param_1;
}

