
void FPU_AsinWrapper(void)

{
  float10 in_ST0;
  double dVar1;
  
  dVar1 = (double)in_ST0;
  FPU_GetDoubleExponent(SUB84(dVar1,0),(uint)((ulonglong)dVar1 >> 0x20));
  FPU_AsinHelper(SUB84(dVar1,0),(uint)((ulonglong)dVar1 >> 0x20));
  return;
}

