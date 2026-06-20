
int D3DX_LoadTextureMemory
              (int *param_1,undefined4 param_2,uint *param_3,int *param_4,undefined4 param_5,
              uint *param_6,uint param_7,undefined4 param_8)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_2c [9];
  int local_8;
  
  IntZero_4(&local_8);
  if ((param_1 == (int *)0x0) || (param_4 == (int *)0x0)) {
    iVar3 = -0x7789f794;
  }
  else {
    uVar1 = 1;
    if (param_1 == param_4) {
      uVar1 = 0x10001;
    }
    iVar2 = Texture_UpdateRegion(&local_8,&local_54,param_4,param_5,param_6,0,uVar1);
    if (-1 < iVar2) {
      iVar2 = D3DX_CreateTextureFromMemory
                        (param_1,param_2,param_3,local_54,local_50,local_4c,local_48,param_5,
                         local_2c,param_7,param_8);
      iVar3 = 0;
      if (-1 < iVar2) goto LAB_00488871;
    }
    iVar3 = iVar2;
  }
LAB_00488871:
  D3DX_ReleaseSurface(&local_8);
  return iVar3;
}

