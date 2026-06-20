
void FPU_DivideThunk(void)

{
  float10 in_ST0;
  float10 in_ST1;
  
  CRT_PowDouble((double)in_ST1,SUB84((double)in_ST0,0),(uint)((ulonglong)(double)in_ST0 >> 0x20));
  return;
}

