
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FPU_RoundCheck(void)

{
  float10 in_ST0;
  
  if (ROUND(in_ST0) == in_ST0) {
    return;
  }
  return;
}

