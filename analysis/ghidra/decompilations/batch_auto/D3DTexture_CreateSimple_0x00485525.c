
int D3DTexture_CreateSimple
              (uint param_1,uint param_2,uint param_3,uint param_4,int *param_5,undefined4 *param_6)

{
  void *pvVar1;
  int iVar2;
  void *this;
  
  if ((((((param_3 & 0xfffe3000) == 0) && (this = (void *)0x0, param_5 != (int *)0x0)) &&
       (param_6 != (undefined4 *)0x0)) && ((param_4 != 0 && ((param_4 & 1) == 0)))) &&
     ((((byte)param_4 & 0xe) != 4 && ((param_1 != 0 && (param_2 != 0)))))) {
    *param_6 = 0;
    if ((param_3 & 1) == 0) {
      if ((0xffff < param_1) || (0xffff < param_2)) goto LAB_00485604;
      pvVar1 = operator_new(0x70);
      if (pvVar1 != (void *)0x0) {
        this = D3DTexture_InitLocked(pvVar1,param_5,param_4,param_3);
      }
      if (this == (void *)0x0) {
        return -0x7ff8fff2;
      }
      iVar2 = MeshBuffer_Allocate(this,param_1,param_2);
    }
    else {
      pvVar1 = operator_new(0x70);
      if (pvVar1 != (void *)0x0) {
        this = D3DTexture_Init(pvVar1,param_5,param_4,param_3);
      }
      if (this == (void *)0x0) {
        return -0x7ff8fff2;
      }
      iVar2 = D3DTexture_ResizeAndValidate(this,param_1,param_2);
    }
    if (-1 < iVar2) {
      *param_6 = this;
    }
  }
  else {
LAB_00485604:
    iVar2 = -0x7789f794;
  }
  return iVar2;
}

