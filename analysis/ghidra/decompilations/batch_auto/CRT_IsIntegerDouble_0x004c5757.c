
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __cdecl CRT_IsIntegerDouble(double param_1)

{
  double dVar1;
  uint uVar2;
  float10 fVar3;
  
  uVar2 = FPU_ClassifyDouble(SUB84(param_1,0),(uint)((ulonglong)param_1 >> 0x20));
  if ((uVar2 & 0x90) == 0) {
    fVar3 = FPU_Round(param_1);
    if ((double)fVar3 == param_1) {
      dVar1 = param_1 / _DAT_004cf4f8;
      fVar3 = FPU_Round(dVar1);
      if (fVar3 == (float10)dVar1) {
        return 2;
      }
      return 1;
    }
  }
  return 0;
}

