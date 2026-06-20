
int D3DX_CreateTextureFromResource
              (int *param_1,undefined4 param_2,uint *param_3,int param_4,undefined4 param_5,
              undefined4 param_6,undefined4 param_7,undefined4 *param_8,uint param_9,
              undefined4 param_10)

{
  int iVar1;
  int extraout_EAX;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 local_b8 [19];
  int local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c [4];
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44 [7];
  undefined4 local_28;
  undefined4 local_24;
  byte local_20 [16];
  int local_10 [3];
  
  D3DX_PairInit(local_10);
  Rect4_Zero((undefined4 *)local_20);
  if (param_1 == (int *)0x0) {
    iVar2 = -0x7789f794;
  }
  else if ((param_4 == 0) || (param_8 == (undefined4 *)0x0)) {
    iVar2 = -0x7789f794;
  }
  else {
    if (param_9 == 0xffffffff) {
      param_9 = 0x80004;
    }
    iVar1 = D3DX_LoadSurfaceFromSurface(local_20,local_b8,param_1,param_2,param_3,0,0);
    if (-1 < iVar1) {
      local_68 = param_5;
      local_6c = param_4;
      local_64 = param_6;
      local_60 = 0;
      local_5c[0] = *param_8;
      local_5c[1] = param_8[1];
      local_5c[2] = param_8[2];
      local_5c[3] = param_8[3];
      local_4c = 0;
      local_48 = 1;
      puVar3 = local_5c;
      puVar4 = local_44;
      for (iVar2 = 6; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar4 = *puVar3;
        puVar3 = puVar3 + 1;
        puVar4 = puVar4 + 1;
      }
      local_28 = param_10;
      local_24 = param_7;
      iVar2 = D3DX_InitSurfaceLoader(local_10,(int)local_b8,&local_6c,param_9);
      iVar1 = extraout_EAX;
      if (iVar2 <= extraout_EAX) goto LAB_0048823d;
    }
    iVar2 = iVar1;
  }
LAB_0048823d:
  D3DX_ReleaseSurfaces(local_20);
  D3DX_PairDtor(local_10);
  return iVar2;
}

