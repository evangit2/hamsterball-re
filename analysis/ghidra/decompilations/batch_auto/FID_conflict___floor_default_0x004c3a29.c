
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Library Function - Multiple Matches With Different Base Names
    __ceil_default
    __floor_default
   
   Library: Visual Studio 2005 Release */

float10 __cdecl FID_conflict___floor_default(double param_1)

{
  double dVar1;
  uint uVar2;
  int iVar3;
  float10 fVar4;
  uint uVar5;
  
  uVar2 = __ctrlfp();
  if ((param_1._6_2_ & 0x7ff0) == 0x7ff0) {
    iVar3 = __sptype(SUB84(param_1,0),(uint)((ulonglong)param_1 >> 0x20));
    if (0 < iVar3) {
      if (iVar3 < 3) {
        __ctrlfp();
        return (float10)param_1;
      }
      if (iVar3 == 3) {
        fVar4 = __handle_qnan1(0xc,param_1);
        return fVar4;
      }
    }
    dVar1 = param_1 + _DAT_004cf3c8;
    uVar5 = 8;
  }
  else {
    fVar4 = FPU_Round(param_1);
    dVar1 = (double)fVar4;
    if ((dVar1 == param_1) || ((uVar2 & 0x20) != 0)) {
      __ctrlfp();
      return (float10)dVar1;
    }
    uVar5 = 0x10;
  }
  fVar4 = FPU_RoundAndHandleError(uVar5,0xc,param_1,dVar1,uVar2);
  return fVar4;
}

