
undefined4 __thiscall
D3DX_LoadTextureFromMemory(void *this,uint *param_1,int *param_2,int *param_3,int param_4)

{
  int *piVar1;
  int *extraout_EAX;
  undefined4 uVar2;
  int iVar3;
  int *piVar4;
  int local_24 [8];
  
  local_24[0] = 0;
  local_24[1] = 5;
  local_24[2] = 4;
  local_24[3] = 1;
  local_24[4] = 3;
  local_24[5] = 2;
  local_24[6] = 6;
  local_24[7] = 0;
  piVar1 = param_2;
  do {
    *(int *)((int)this + 0x40) = param_4;
    iVar3 = local_24[local_24[7]];
    *(int *)((int)this + 0x48) = iVar3;
    *(int *)((int)this + 0x44) = 3;
    if (iVar3 == 0) {
      piVar1 = (int *)D3DX_ParseBMPPixelFormat(this,(short *)param_1,(uint)param_2);
    }
    else if (iVar3 == 1) {
      piVar1 = (int *)D3DX_LoadSurfaceFromMemory(param_1,param_2);
    }
    else if (iVar3 == 2) {
      piVar1 = (int *)D3DX_ParseDDSurfaceDesc(this,param_1,(uint)param_2);
    }
    else if (iVar3 == 3) {
      this = (void *)D3DX_LoadSurfaceFromPNG((int)param_1,(uint)param_2);
      piVar1 = extraout_EAX;
    }
    else if (iVar3 == 4) {
      piVar1 = (int *)DDTexture_CreateMipChain(this,(int *)param_1,param_2);
    }
    else if (iVar3 == 5) {
      piVar1 = (int *)D3DX_ParsePPM(this,param_1,(uint)param_2);
    }
    else if (iVar3 == 6) {
      piVar1 = (int *)DDSURFACEDESC_ParsePixelFormat(this,param_1,(uint)param_2);
    }
    if (-1 < (int)piVar1) break;
    if ((*(void **)((int)this + 4) != (void *)0x0) && (*(int *)((int)this + 0x38) != 0)) {
      _free(*(void **)((int)this + 4));
    }
    if ((*(void **)((int)this + 8) != (void *)0x0) && (*(int *)((int)this + 0x3c) != 0)) {
      _free(*(void **)((int)this + 8));
    }
    if (*(void **)((int)this + 0x4c) != (void *)0x0) {
      TextureCache_DeletingDtor(*(void **)((int)this + 0x4c),1);
    }
    if (*(void **)((int)this + 0x50) != (void *)0x0) {
      TextureCache_DeletingDtor(*(void **)((int)this + 0x50),1);
    }
    local_24[7] = local_24[7] + 1;
    *(int *)((int)this + 4) = 0;
    *(int *)((int)this + 8) = 0;
    *(int *)((int)this + 0x38) = 0;
    *(int *)((int)this + 0x3c) = 0;
    *(int *)((int)this + 0x4c) = 0;
    *(int *)((int)this + 0x50) = 0;
  } while ((uint)local_24[7] < 7);
  iVar3 = 7;
  if (local_24[7] == 7) {
    uVar2 = 0x88760b59;
  }
  else {
    piVar1 = param_3;
    piVar4 = this;
    if (param_3 != (int *)0x0) {
      for (; iVar3 != 0; iVar3 = iVar3 + -1) {
        *piVar1 = 0;
        piVar1 = piVar1 + 1;
      }
      *param_3 = *(int *)((int)this + 0xc);
      param_3[1] = *(int *)((int)this + 0x10);
      param_3[2] = *(int *)((int)this + 0x14);
      param_3[3] = 1;
      iVar3 = D3D_ConvertFourCC(*(int *)this);
      param_3[4] = iVar3;
      param_3[5] = *(int *)((int)this + 0x44);
      param_3[6] = *(int *)((int)this + 0x48);
      for (piVar1 = this; piVar1[0x13] != 0; piVar1 = (int *)piVar1[0x13]) {
        param_3[3] = param_3[3] + 1;
      }
    }
    do {
      for (; this != (int *)0x0; this = *(void **)((int)this + 0x4c)) {
        *(int *)((int)this + 0x20) = *(int *)((int)this + 0xc);
        *(int *)((int)this + 0x24) = *(int *)((int)this + 0x10);
        *(int *)((int)this + 0x18) = 0;
        *(int *)((int)this + 0x1c) = 0;
        *(int *)((int)this + 0x28) = 0;
        *(int *)((int)this + 0x2c) = *(int *)((int)this + 0x14);
      }
      this = (void *)piVar4[0x14];
      piVar4 = this;
    } while (this != (int *)0x0);
    uVar2 = 0;
  }
  return uVar2;
}

