
undefined4
D3D_ThunkShaderDispatch4
          (undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined1 local_14 [16];
  
  D3DX_ShaderDispatch_noarg2(local_14,param_2,param_3,param_4);
  D3DThunk_AcquireThreadFocus();
  return param_1;
}

