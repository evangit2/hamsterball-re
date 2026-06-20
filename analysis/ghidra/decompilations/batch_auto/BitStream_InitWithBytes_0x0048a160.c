
undefined4 __cdecl BitStream_InitWithBytes(undefined4 *param_1,undefined4 param_2)

{
  OggStream_Reset(param_1);
  param_1[6] = param_2;
  return 0;
}

