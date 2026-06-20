
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl PNG_ParseSRGBChunk(int *param_1,int param_2,uint param_3)

{
  uint uVar1;
  int *piVar2;
  byte bVar3;
  int iVar4;
  char *pcVar5;
  
  piVar2 = param_1;
  uVar1 = param_1[0x16];
  if ((uVar1 & 1) == 0) {
    longjmp_with_cleanup(param_1,"Missing IHDR before sRGB");
  }
  else {
    if ((uVar1 & 4) != 0) {
      pcVar5 = "Invalid sRGB after IDAT";
      goto LAB_004ae433;
    }
    if ((uVar1 & 2) == 0) {
      if ((param_2 != 0) && ((*(byte *)(param_2 + 9) & 8) != 0)) {
        pcVar5 = "Duplicate sRGB chunk";
        goto LAB_004ae433;
      }
    }
    else {
      seh_filter_invoke((int)param_1,"Out of place sRGB chunk");
    }
  }
  if (param_3 == 1) {
    Zlib_UpdateHash(piVar2,(byte *)((int)&param_1 + 3),1);
    iVar4 = Zlib_FlushWithCRC(piVar2,0);
    if (iVar4 != 0) {
      return;
    }
    bVar3 = param_1._3_1_;
    if (3 < param_1._3_1_) {
      seh_filter_invoke((int)piVar2,"Unknown sRGB intent");
      return;
    }
    if (((*(byte *)(param_2 + 8) & 1) != 0) &&
       ((float)_DAT_004e6a78 <
        ABS(((float)piVar2[0x4c] * _DAT_004e6b08 + (float)_DAT_004cf3e0) - (float)_DAT_004e6a80))) {
      seh_filter_invoke((int)piVar2,"Ignoring incorrect gAMA value when sRGB is also present");
    }
    PNG_SetSRGBWithGamma((int)piVar2,param_2,bVar3);
    return;
  }
  pcVar5 = "Incorrect sRGB chunk length";
LAB_004ae433:
  seh_filter_invoke((int)piVar2,pcVar5);
  Zlib_FlushWithCRC(piVar2,param_3);
  return;
}

