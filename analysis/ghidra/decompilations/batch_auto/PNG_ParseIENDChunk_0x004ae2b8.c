
void __cdecl
PNG_ParseIENDChunk(int *param_1,undefined4 param_2,undefined4 param_3,int param_4,uint param_5)

{
  if ((((param_1[0x16] & 1U) == 0) || ((param_1[0x16] & 4U) == 0)) &&
     (longjmp_with_cleanup(param_1,"No image in file"), param_4 == 0)) {
    return;
  }
  param_1[0x16] = param_1[0x16] | 0x18;
  if (param_5 != 0) {
    seh_filter_invoke((int)param_1,"Incorrect IEND chunk length");
  }
  Zlib_FlushWithCRC(param_1,param_5);
  return;
}

