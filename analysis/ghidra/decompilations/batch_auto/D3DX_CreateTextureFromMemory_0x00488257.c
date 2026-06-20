
int D3DX_CreateTextureFromMemory
              (int *param_1,undefined4 param_2,uint *param_3,int param_4,undefined4 param_5,
              undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 *param_9,
              uint param_10,undefined4 param_11)

{
  int extraout_EAX;
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 local_ac [19];
  int local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50 [6];
  undefined4 local_38 [7];
  undefined4 local_1c;
  undefined4 local_18;
  int local_14 [3];
  int local_8;
  
  D3DX_PairInit(local_14);
  IntZero_4(&local_8);
  if (((param_1 == (int *)0x0) || (param_4 == 0)) || (param_9 == (undefined4 *)0x0)) {
    iVar1 = -0x7789f794;
  }
  else {
    if (param_10 == 0xffffffff) {
      param_10 = 0x80004;
    }
    iVar1 = Texture_UpdateRegion(&local_8,local_ac,param_1,param_2,param_3,0,0);
    if (-1 < iVar1) {
      local_5c = param_5;
      local_58 = param_6;
      local_60 = param_4;
      local_54 = param_7;
      puVar2 = param_9;
      puVar3 = local_50;
      for (iVar1 = 6; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = *puVar2;
        puVar2 = puVar2 + 1;
        puVar3 = puVar3 + 1;
      }
      puVar2 = local_38;
      for (iVar1 = 6; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar2 = *param_9;
        param_9 = param_9 + 1;
        puVar2 = puVar2 + 1;
      }
      local_1c = param_11;
      local_18 = param_8;
      D3DX_InitSurfaceLoader(local_14,(int)local_ac,&local_60,param_10);
      iVar1 = extraout_EAX;
      if (-1 < extraout_EAX) {
        iVar1 = 0;
      }
    }
  }
  D3DX_ReleaseSurface(&local_8);
  D3DX_PairDtor(local_14);
  return iVar1;
}

