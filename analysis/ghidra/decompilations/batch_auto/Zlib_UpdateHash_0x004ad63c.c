
void __cdecl Zlib_UpdateHash(int *param_1,byte *param_2,uint param_3)

{
  FileStream_ReadCallback(param_1,param_2,param_3);
  PNG_UpdateCRC32((int)param_1,param_2,param_3);
  return;
}

