
void __cdecl PNG_ParseFrame(int *param_1,uint *param_2)

{
  byte *pbVar1;
  int *piVar2;
  int iVar3;
  int unaff_EBX;
  uint uVar4;
  uint unaff_EBP;
  uint uVar5;
  char *pcVar6;
  
  piVar2 = param_1;
  if (*(byte *)(param_1 + 0x47) < 8) {
    uVar4 = (uint)*(byte *)(param_1 + 0x47);
    uVar5 = -uVar4 + 8;
    FileStream_ReadCallback(param_1,uVar4 + 0x20 + (int)param_2,uVar5);
    *(undefined1 *)(piVar2 + 0x47) = 8;
    iVar3 = PNG_CompareSignature((int)(param_2 + 8),uVar4,uVar5);
    if (iVar3 != 0) {
      if ((uVar4 < 4) &&
         (iVar3 = PNG_CompareSignature((int)(param_2 + 8),uVar4,-uVar4 + 4), iVar3 != 0)) {
        pcVar6 = "Not a PNG file";
      }
      else {
        pcVar6 = "PNG file corrupted by ASCII conversion";
      }
      longjmp_with_cleanup(piVar2,pcVar6);
    }
  }
  pbVar1 = (byte *)(piVar2 + 0x43);
  while( true ) {
    while( true ) {
      while( true ) {
        while( true ) {
          FileStream_ReadCallback(piVar2,&param_1,4);
          uVar4 = ReadInt32BE((undefined1 *)&param_1);
          PNG_InitCRC32((int)piVar2);
          Zlib_UpdateHash(piVar2,pbVar1,4);
          if (*(int *)pbVar1 != DAT_004e56a4) break;
          PNG_ParseIHDRChunk(piVar2,param_2,uVar4);
        }
        if (*(void **)pbVar1 != DAT_004e56bc) break;
        PNG_ParsePLTE(*(void **)pbVar1,piVar2,(int)param_2,uVar4);
      }
      if (*(int *)pbVar1 != DAT_004e56b4) break;
      PNG_ParseIENDChunk(piVar2,param_2,uVar4,unaff_EBX,unaff_EBP);
    }
    if (*(int *)pbVar1 == DAT_004e56ac) break;
    if (*(int *)pbVar1 == DAT_004e56c4) {
      PNG_ParsegAMAChunk(piVar2,(int)param_2,uVar4);
    }
    else if (*(int *)pbVar1 == DAT_004e56cc) {
      PNG_ParseSRGBChunk(piVar2,(int)param_2,uVar4);
    }
    else if (*(int *)pbVar1 == DAT_004e56d4) {
      PNG_ParseTRNSChunk(piVar2,(int)param_2,uVar4);
    }
    else {
      PNG_ParseUnknownChunk(piVar2,(int)param_2,uVar4);
    }
  }
  if ((piVar2[0x16] & 1U) == 0) {
    pcVar6 = "Missing IHDR before IDAT";
  }
  else {
    if ((*(char *)((int)piVar2 + 0x116) != '\x03') || ((piVar2[0x16] & 2U) != 0)) goto LAB_004a4870;
    pcVar6 = "Missing PLTE before IDAT";
  }
  longjmp_with_cleanup(piVar2,pcVar6);
LAB_004a4870:
  piVar2[0x16] = piVar2[0x16] | 4;
  piVar2[0x3f] = uVar4;
  return;
}

