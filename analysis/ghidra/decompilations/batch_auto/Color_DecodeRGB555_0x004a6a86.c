
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Color_DecodeRGB555(uint param_1)

{
  float fVar1;
  float *in_EAX;
  
  fVar1 = _DAT_004d5c50;
  *in_EAX = (float)(param_1 >> 0xb & 0x1f) * _DAT_004d5c50;
  in_EAX[1] = (float)(param_1 >> 5 & 0x3f) * _DAT_004dc08c;
  in_EAX[2] = (float)(param_1 & 0x1f) * fVar1;
  in_EAX[3] = 1.0;
  return;
}

