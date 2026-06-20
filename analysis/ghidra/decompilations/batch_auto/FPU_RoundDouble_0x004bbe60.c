
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __cdecl FPU_RoundDouble(double param_1,undefined2 param_2)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  ushort in_FPUControlWord;
  float10 fVar4;
  double dVar5;
  longlong lVar6;
  undefined8 uVar7;
  
  if ((DAT_00535980 != 0) && ((MXCSR & 0x1f80) == 0x1f80 && (in_FPUControlWord & 0x7f) == 0x7f)) {
    uVar2 = (uint)((ulonglong)param_1 >> 0x20);
    uVar1 = uVar2 >> 0x14;
    lVar6 = CONCAT44(_UNK_004e9ae4,_DAT_004e9ae0 - (uVar2 >> 0x14 & _DAT_004e9b10));
    if ((uVar1 & 0x800) == 0) {
      if (uVar1 < 0x3ff) {
        return (float10)0;
      }
      if (uVar1 < 0x433) {
        return (float10)(double)(((ulonglong)param_1 >> lVar6) << lVar6);
      }
    }
    else {
      dVar5 = (double)(((ulonglong)param_1 >> lVar6) << lVar6);
      if (uVar1 < 0xbff) {
        return (float10)(double)((-(ulonglong)(param_1 < _DAT_004e9b00) | (ulonglong)_DAT_004e9b00)
                                & _DAT_004e9af0);
      }
      if (uVar1 < 0xc33) {
        return (float10)(dVar5 - (double)(-(ulonglong)(param_1 < dVar5) & _DAT_004e9ad0));
      }
    }
    if (NAN(param_1)) {
      FPU_WriteMathError(&param_1,&param_1,&param_1,0x3ed);
    }
    return (float10)(double)CONCAT26(param_1._6_2_,param_1._0_6_);
  }
  uVar2 = __ctrlfp();
  uVar1 = (uint)(CONCAT26(param_1._6_2_,param_1._0_6_) >> 0x20);
  if ((param_1._6_2_ & 0x7ff0) == 0x7ff0) {
    iVar3 = __sptype((int)param_1._0_6_,uVar1);
    if (0 < iVar3) {
      if (iVar3 < 3) {
        __ctrlfp();
        return (float10)(double)CONCAT26(param_1._6_2_,param_1._0_6_);
      }
      if (iVar3 == 3) {
        fVar4 = __handle_qnan1(0xb,(double)CONCAT44((int)(CONCAT26(param_1._6_2_,param_1._0_6_) >>
                                                         0x20),(int)param_1._0_6_));
        return fVar4;
      }
    }
    dVar5 = (double)CONCAT26(param_1._6_2_,param_1._0_6_) + _DAT_004cf3c8;
    uVar7 = CONCAT26(param_1._6_2_,param_1._0_6_);
    uVar1 = 8;
  }
  else {
    fVar4 = FPU_Round((double)CONCAT44(uVar1,(int)param_1._0_6_));
    dVar5 = (double)fVar4;
    if ((dVar5 == (double)CONCAT26(param_1._6_2_,param_1._0_6_)) || ((uVar2 & 0x20) != 0)) {
      __ctrlfp();
      return (float10)dVar5;
    }
    uVar7 = CONCAT26(param_1._6_2_,param_1._0_6_);
    uVar1 = 0x10;
  }
  fVar4 = FPU_RoundAndHandleError(uVar1,0xb,uVar7,dVar5,uVar2);
  return fVar4;
}

