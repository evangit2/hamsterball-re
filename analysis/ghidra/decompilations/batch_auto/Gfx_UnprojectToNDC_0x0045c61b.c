
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float * Gfx_UnprojectToNDC(float *param_1,float *param_2,int *param_3,int param_4,int param_5,
                          int param_6)

{
  float fVar1;
  float fVar2;
  
  switch(((param_6 != 0) << 1 | param_5 != 0) << 1 | param_4 != 0) {
  case '\0':
    goto switchD_0045c658_default;
  case '\x01':
    goto LAB_0045c6f6;
  case '\x02':
    goto LAB_0045c6f6;
  case '\x03':
    break;
  case '\x04':
    goto LAB_0045c6f6;
  case '\x05':
    break;
  case '\x06':
    break;
  case '\a':
    Graphics_SetRenderState();
    break;
  default:
    goto switchD_0045c658_default;
  }
  Graphics_SetRenderState();
LAB_0045c6f6:
  D3D_Thunk_5();
switchD_0045c658_default:
  if (param_3 != (int *)0x0) {
    fVar1 = (float)*param_3;
    if (*param_3 < 0) {
      fVar1 = fVar1 + _DAT_004cf558;
    }
    fVar2 = (float)param_3[2];
    if (param_3[2] < 0) {
      fVar2 = fVar2 + _DAT_004cf558;
    }
    fVar2 = (*param_2 - fVar1) / fVar2;
    *param_1 = (fVar2 + fVar2) - _DAT_004cf310;
    fVar1 = (float)param_3[1];
    if (param_3[1] < 0) {
      fVar1 = fVar1 + _DAT_004cf558;
    }
    fVar2 = (float)param_3[3];
    if (param_3[3] < 0) {
      fVar2 = fVar2 + _DAT_004cf558;
    }
    fVar2 = (param_2[1] - fVar1) / fVar2;
    param_1[1] = -((fVar2 + fVar2) - _DAT_004cf310);
    param_1[2] = (param_2[2] - (float)param_3[4]) / ((float)param_3[5] - (float)param_3[4]);
  }
  D3DX_ShaderDispatch1();
  return param_1;
}

