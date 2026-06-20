
byte * CRT_ScanMBCSWhitespace(void)

{
  byte bVar1;
  int iVar2;
  byte *pbVar3;
  
  if (DAT_00536aec == 0) {
    ___initmbctable();
  }
  if (DAT_00536af0 == (byte *)0x0) {
    pbVar3 = &DAT_004d1354;
  }
  else {
    bVar1 = *DAT_00536af0;
    pbVar3 = DAT_00536af0;
    if (bVar1 != 0x22) {
      do {
        if (bVar1 < 0x21) goto LAB_004bfb57;
        bVar1 = pbVar3[1];
        pbVar3 = pbVar3 + 1;
      } while( true );
    }
    pbVar3 = DAT_00536af0 + 1;
    bVar1 = *pbVar3;
    if (bVar1 != 0x22) {
      do {
        if (bVar1 == 0) break;
        iVar2 = CRT_IsAlphaNumericThunk(bVar1);
        if (iVar2 != 0) {
          pbVar3 = pbVar3 + 1;
        }
        pbVar3 = pbVar3 + 1;
        bVar1 = *pbVar3;
      } while (bVar1 != 0x22);
      if (*pbVar3 != 0x22) goto LAB_004bfb57;
    }
    do {
      pbVar3 = pbVar3 + 1;
LAB_004bfb57:
    } while ((*pbVar3 != 0) && (*pbVar3 < 0x21));
  }
  return pbVar3;
}

