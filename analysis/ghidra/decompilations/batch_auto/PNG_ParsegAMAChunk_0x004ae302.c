
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl PNG_ParsegAMAChunk(int *param_1,int param_2,uint param_3)

{
  uint uVar1;
  float fVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  char *pcVar6;
  
  iVar4 = param_2;
  piVar3 = param_1;
  uVar1 = param_1[0x16];
  if ((uVar1 & 1) == 0) {
    longjmp_with_cleanup(param_1,"Missing IHDR before gAMA");
  }
  else {
    if ((uVar1 & 4) != 0) {
      pcVar6 = "Invalid gAMA after IDAT";
      goto LAB_004ae333;
    }
    if ((uVar1 & 2) == 0) {
      if (((param_2 != 0) && ((*(uint *)(param_2 + 8) & 1) != 0)) &&
         ((*(uint *)(param_2 + 8) & 0x800) == 0)) {
        pcVar6 = "Duplicate gAMA chunk";
        goto LAB_004ae333;
      }
    }
    else {
      seh_filter_invoke((int)param_1,"Out of place gAMA chunk");
    }
  }
  if (param_3 == 4) {
    Zlib_UpdateHash(piVar3,(byte *)&param_1,4);
    iVar5 = Zlib_FlushWithCRC(piVar3,0);
    if (iVar5 != 0) {
      return;
    }
    iVar5 = ReadInt32BE((undefined1 *)&param_1);
    if (iVar5 == 0) {
      return;
    }
    if ((*(uint *)(iVar4 + 8) & 0x800) != 0) {
      fVar2 = (float)iVar5;
      if (iVar5 < 0) {
        fVar2 = fVar2 + _DAT_004cf558;
      }
      if ((float)_DAT_004e6a78 < ABS(fVar2 - (float)_DAT_004e6a80)) {
        seh_filter_invoke((int)piVar3,"Ignoring incorrect gAMA value when sRGB is also present");
        return;
      }
    }
    fVar2 = (float)iVar5;
    if (iVar5 < 0) {
      fVar2 = fVar2 + _DAT_004cf558;
    }
    fVar2 = fVar2 * _DAT_004dc040;
    piVar3[0x4c] = (int)fVar2;
    PNG_SetGamma((int)piVar3,iVar4,(double)fVar2);
    return;
  }
  pcVar6 = "Incorrect gAMA chunk length";
LAB_004ae333:
  seh_filter_invoke((int)piVar3,pcVar6);
  Zlib_FlushWithCRC(piVar3,param_3);
  return;
}

