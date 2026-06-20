
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Library Function - Single Match
    __powhlp
   
   Library: Visual Studio 2003 Release */

undefined4 __cdecl __powhlp(int param_1,int param_2,double param_3,double *param_4)

{
  double dVar1;
  double dVar2;
  int iVar3;
  undefined4 uVar4;
  
  dVar1 = (double)CONCAT44(param_2,param_1);
  uVar4 = 0;
  if (dVar1 < _DAT_004cf6a8) {
    dVar1 = -dVar1;
  }
  dVar2 = _DAT_004fcd60;
  if (param_3._4_4_ == 0x7ff00000) {
    if (param_3._0_4_ != 0) goto LAB_004c586c;
    if (_DAT_004cf3c8 < dVar1) goto LAB_004c591c;
    if (_DAT_004cf3c8 <= dVar1) goto LAB_004c581f;
  }
  else {
    if (param_3 == -INFINITY) {
      if (_DAT_004cf3c8 < dVar1) {
        dVar2 = 0.0;
        goto LAB_004c591c;
      }
      if (dVar1 < _DAT_004cf3c8) goto LAB_004c591c;
LAB_004c581f:
      uVar4 = 1;
      dVar2 = _DAT_004fcd68;
      goto LAB_004c591c;
    }
LAB_004c586c:
    if (param_2 != 0x7ff00000) {
      if (param_2 != -0x100000) {
        return 0;
      }
      if (param_1 != 0) {
        return 0;
      }
      iVar3 = CRT_IsIntegerDouble(param_3);
      if (param_3 <= _DAT_004cf6a8) {
        if (_DAT_004cf6a8 <= param_3) {
          dVar2 = 1.0;
        }
        else {
          dVar2 = _DAT_004fcd80;
          if (iVar3 != 1) {
            dVar2 = 0.0;
          }
        }
      }
      else {
        dVar2 = _DAT_004fcd60;
        if (iVar3 == 1) {
          dVar2 = -_DAT_004fcd60;
        }
      }
      goto LAB_004c591c;
    }
    if (param_1 != 0) {
      return 0;
    }
    if (_DAT_004cf6a8 < param_3) goto LAB_004c591c;
    if (_DAT_004cf6a8 <= param_3) {
      dVar2 = 1.0;
      goto LAB_004c591c;
    }
  }
  dVar2 = 0.0;
LAB_004c591c:
  *param_4 = dVar2;
  return uVar4;
}

