
void __cdecl PNG_SetupRowTransform(int *param_1)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  void *pvVar4;
  undefined4 *puVar5;
  int iVar6;
  
  param_1[0x1a] = 0;
  PNG_CorrectPaletteGamma(param_1);
  uVar3 = param_1[0x2f];
  if (*(char *)((int)param_1 + 0x113) == '\0') {
    iVar6 = param_1[0x2e];
    param_1[0x30] = uVar3;
    param_1[0x34] = iVar6;
    param_1[0x33] = param_1[0x32] + 1;
  }
  else {
    if ((*(byte *)(param_1 + 0x18) & 2) == 0) {
      uVar3 = uVar3 + 7 >> 3;
    }
    iVar6 = param_1[0x2e];
    param_1[0x30] = uVar3;
    uVar3 = ((iVar6 - *(int *)(&DAT_004e67b0 + (uint)*(byte *)(param_1 + 0x45) * 4)) + -1 +
            *(uint *)(&DAT_004e67cc + (uint)*(byte *)(param_1 + 0x45) * 4)) /
            *(uint *)(&DAT_004e67cc + (uint)*(byte *)(param_1 + 0x45) * 4);
    param_1[0x34] = uVar3;
    param_1[0x33] = (*(byte *)((int)param_1 + 0x119) * uVar3 + 7 >> 3) + 1;
  }
  uVar3 = (uint)*(byte *)((int)param_1 + 0x119);
  uVar2 = param_1[0x18];
  if (((uVar2 & 4) != 0) && (*(byte *)((int)param_1 + 0x117) < 8)) {
    uVar3 = 8;
  }
  if ((uVar2 & 0x1000) != 0) {
    cVar1 = *(char *)((int)param_1 + 0x116);
    if (cVar1 == '\x03') {
      uVar3 = (uint)(*(short *)((int)param_1 + 0x10a) != 0) * 8 + 0x18;
    }
    else if (cVar1 == '\0') {
      if (uVar3 < 8) {
        uVar3 = 8;
      }
      if (*(short *)((int)param_1 + 0x10a) != 0) {
        uVar3 = uVar3 * 2;
      }
    }
    else if ((cVar1 == '\x02') && (*(short *)((int)param_1 + 0x10a) != 0)) {
      uVar3 = (uVar3 << 2) / 3;
    }
  }
  if ((char)(uVar2 >> 8) < '\0') {
    cVar1 = *(char *)((int)param_1 + 0x116);
    if (cVar1 == '\x03') {
      uVar3 = 0x20;
    }
    else if (cVar1 == '\0') {
      uVar3 = ((8 < uVar3) - 1 & 0xfffffff0) + 0x20;
    }
    else if (cVar1 == '\x02') {
      uVar3 = ((0x20 < uVar3) - 1 & 0xffffffe0) + 0x40;
    }
  }
  pvVar4 = Malloc_OrLongjmp(param_1,((iVar6 + 7U & 0xfffffff8) * uVar3 + 7 >> 3) + 1 +
                                    ((int)(uVar3 + 7) >> 3));
  param_1[0x37] = (int)pvVar4;
  puVar5 = Malloc_OrLongjmp(param_1,param_1[0x32] + 1);
  param_1[0x36] = (int)puVar5;
  Zlib_MemSet(param_1,puVar5,0,param_1[0x32] + 1);
  param_1[0x17] = param_1[0x17] | 0x40;
  return;
}

