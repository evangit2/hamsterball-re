
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl PNG_SetSRGBWithGamma(int param_1,int param_2,undefined1 param_3)

{
  if ((param_1 != 0) && (param_2 != 0)) {
    PNG_SetSRGB(param_1,param_2,param_3);
    PNG_SetGamma(param_1,param_2,_DAT_004e58f8);
  }
  return;
}

