
undefined4 __fastcall Color_NormalizeByAlpha(float *param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  
  iVar3 = 0x10;
  do {
    if (param_1[3] == 0.0) {
      *param_1 = 0.0;
      param_1[1] = 0.0;
      fVar1 = 0.0;
LAB_004a6b44:
      param_1[2] = fVar1;
    }
    else if (param_1[3] < 1.0) {
      fVar1 = 1.0 / param_1[3];
      if (param_1[3] <= *param_1) {
        fVar2 = 1.0;
      }
      else {
        fVar2 = fVar1 * *param_1;
      }
      *param_1 = fVar2;
      if (param_1[3] <= param_1[1]) {
        fVar2 = 1.0;
      }
      else {
        fVar2 = fVar1 * param_1[1];
      }
      param_1[1] = fVar2;
      if (param_1[3] <= param_1[2]) {
        fVar1 = 1.0;
      }
      else {
        fVar1 = fVar1 * param_1[2];
      }
      goto LAB_004a6b44;
    }
    param_1 = param_1 + 4;
    iVar3 = iVar3 + -1;
    if (iVar3 == 0) {
      return 0;
    }
  } while( true );
}

