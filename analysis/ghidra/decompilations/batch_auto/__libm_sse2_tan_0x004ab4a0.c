
ulonglong __libm_sse2_tan(void)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  ulonglong in_MM0;
  ulonglong uVar4;
  undefined8 uVar5;
  undefined4 uVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  undefined8 uVar9;
  undefined8 uVar10;
  
  uVar1 = (uint)in_MM0;
  if ((int)(uVar1 ^ uVar1 & 0x80000000) < DAT_004fb254) {
    uVar5 = PackedFloatingMUL(in_MM0 & DAT_004fb19c,(ulonglong)DAT_004fb250);
    uVar4 = PackedFloatingToIntDwordConv(uVar5,uVar5);
    uVar2 = (uint)uVar4;
    uVar5 = PackedIntToFloatingDwordConv(uVar4,uVar4);
    uVar5 = PackedFloatingMUL(CONCAT44((int)uVar5,(int)uVar5),DAT_004fb160);
    uVar7 = PackedFloatingADD(in_MM0 & DAT_004fb19c,uVar5);
    uVar6 = (undefined4)((ulonglong)uVar5 >> 0x20);
    uVar5 = PackedFloatingADD(uVar7,CONCAT44(uVar6,uVar6));
    uVar5 = CONCAT44((int)uVar5,(int)uVar5);
    if ((uVar4 & 1) != 0) {
      uVar5 = PackedFloatingSUBR(uVar5,DAT_004fb170);
    }
    iVar3 = (uVar1 >> 0x1f ^ uVar2 >> 2) << 0x1f;
    uVar7 = PackedFloatingMUL(uVar5,uVar5);
    uVar9 = PackedFloatingMUL(DAT_004fb168,uVar7);
    uVar8 = PackedFloatingMUL(DAT_004fb178,uVar7);
    uVar10 = PackedFloatingADD(uVar9,DAT_004fb118);
    uVar9 = PackedFloatingMUL(DAT_004fb180,uVar7);
    uVar8 = PackedFloatingMUL(uVar10,uVar8);
    uVar10 = PackedFloatingADD(uVar8,DAT_004fb118);
    uVar8 = PackedFloatingMUL(DAT_004fb188,uVar7);
    uVar7 = PackedFloatingMUL(uVar9,uVar10);
    uVar7 = PackedFloatingADD(uVar7,DAT_004fb118);
    uVar7 = PackedFloatingMUL(uVar7,CONCAT44((int)DAT_004fb118,(int)uVar8));
    uVar7 = PackedFloatingADD(uVar7,DAT_004fb118 >> 0x20);
    uVar4 = PackedFloatingMUL(uVar7,CONCAT44((int)uVar5,(int)DAT_004fb118));
    if (((uVar2 ^ uVar2 >> 1) & 1) != 0) {
      uVar4 = CONCAT44((int)uVar4,(int)(uVar4 >> 0x20));
    }
    uVar4 = CONCAT44(iVar3,iVar3) ^ uVar4 ^ (uVar1 & 0x80000000 ^ (uVar2 >> 1) << 0x1f);
  }
  else {
    uVar4 = (ulonglong)DAT_004fb1a8;
  }
  return uVar4;
}

