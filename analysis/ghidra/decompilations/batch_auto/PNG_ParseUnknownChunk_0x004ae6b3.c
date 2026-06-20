
void __cdecl PNG_ParseUnknownChunk(int *param_1,int param_2,uint param_3)

{
  PNG_ValidateChunkType(param_1,(byte *)(param_1 + 0x43));
  if (((*(byte *)(param_1 + 0x43) & 0x20) == 0) &&
     (EH_MediaParser_longjmp(param_1,(undefined4 *)"unknown critical chunk"), param_2 == 0)) {
    return;
  }
  if ((param_1[0x16] & 4U) != 0) {
    param_1[0x16] = param_1[0x16] | 8;
  }
  Zlib_FlushWithCRC(param_1,param_3);
  return;
}

