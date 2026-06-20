
bool __cdecl Zlib_VerifyChecksum(int *param_1)

{
  int *piVar1;
  int iVar2;
  bool bVar3;
  
  piVar1 = param_1;
  bVar3 = true;
  if ((*(byte *)(param_1 + 0x43) & 0x20) == 0) {
    if ((*(byte *)((int)param_1 + 0x5d) & 8) == 0) goto LAB_004ad682;
  }
  else if ((param_1[0x17] & 0x300U) != 0x300) goto LAB_004ad682;
  bVar3 = false;
LAB_004ad682:
  FileStream_ReadCallback(param_1,&param_1,4);
  if (bVar3) {
    iVar2 = ReadInt32BE((undefined1 *)&param_1);
    bVar3 = iVar2 != piVar1[0x40];
  }
  else {
    bVar3 = false;
  }
  return bVar3;
}

