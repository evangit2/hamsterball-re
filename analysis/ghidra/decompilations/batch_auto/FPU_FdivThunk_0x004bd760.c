
void FPU_FdivThunk(void)

{
  ushort in_FPUControlWord;
  float10 in_ST0;
  float10 in_ST1;
  
  if ((DAT_00535980 != 0) && ((MXCSR & 0x1f80) == 0x1f80 && (in_FPUControlWord & 0x7f) == 0x7f)) {
    FPU_DivideThunk();
    return;
  }
  FPU_FdivHelper(SUB84((double)in_ST1,0),(uint)((ulonglong)(double)in_ST1 >> 0x20),
                 SUB84((double)in_ST0,0),(uint)((ulonglong)(double)in_ST0 >> 0x20));
  return;
}

