
int __cdecl CRT_SetFileTranslationMode(uint param_1,int param_2)

{
  byte *pbVar1;
  byte bVar2;
  int *piVar3;
  byte bVar4;
  
  pbVar1 = (byte *)((&DAT_005369e0)[(int)param_1 >> 5] + 4 + (param_1 & 0x1f) * 0x24);
  bVar2 = *pbVar1;
  if (param_2 == 0x8000) {
    bVar4 = bVar2 & 0x7f;
  }
  else {
    if (param_2 != 0x4000) {
      piVar3 = __errno();
      *piVar3 = 0x16;
      return -1;
    }
    bVar4 = bVar2 | 0x80;
  }
  *pbVar1 = bVar4;
  return (-(uint)((bVar2 & 0x80) != 0) & 0xffffc000) + 0x8000;
}

