
void Matrix_BuildFromTanAngles(undefined8 *param_1,undefined8 param_2,undefined8 param_3)

{
  undefined4 uVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  ulonglong uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  undefined4 uVar9;
  undefined8 uVar8;
  undefined8 uVar10;
  undefined8 uVar11;
  undefined4 uVar12;
  undefined8 uVar13;
  
  PackedFloatingMUL(DAT_004dbcf0,param_3);
  PackedFloatingMUL(DAT_004dbcf0,param_2);
  uVar2 = __libm_sse2_tan();
  uVar3 = __libm_sse2_tan();
  uVar4 = __libm_sse2_tan();
  uVar1 = (undefined4)uVar4;
  uVar9 = (undefined4)(uVar4 >> 0x20);
  uVar7 = CONCAT44((int)uVar3,(int)uVar3);
  uVar12 = (undefined4)(uVar3 >> 0x20);
  uVar10 = CONCAT44(uVar12,uVar12);
  uVar5 = PackedFloatingMUL(CONCAT44(uVar1,uVar1) ^ DAT_004dbc80,uVar10);
  uVar6 = PackedFloatingMUL(CONCAT44(uVar1,uVar1),uVar7);
  uVar8 = PackedFloatingMUL(CONCAT44(uVar9,uVar9),uVar7);
  uVar10 = PackedFloatingMUL(CONCAT44(uVar9,uVar9) ^ DAT_004dbc80 >> 0x20,uVar10);
  uVar13 = CONCAT44((int)uVar2,(int)(uVar2 >> 0x20));
  uVar5 = PackedFloatingMUL(uVar5,uVar2);
  uVar7 = PackedFloatingMUL(uVar6,uVar13);
  uVar6 = PackedFloatingMUL(uVar8,uVar13);
  uVar8 = PackedFloatingMUL(uVar10,uVar2);
  uVar5 = PackedFloatingADD(uVar5,uVar6);
  uVar8 = PackedFloatingADD(uVar7,uVar8);
  uVar7 = PackedFloatingADD(uVar5,uVar5);
  uVar6 = PackedFloatingADD(uVar8,uVar8);
  uVar1 = (undefined4)((ulonglong)uVar5 >> 0x20);
  uVar9 = (undefined4)((ulonglong)uVar8 >> 0x20);
  uVar13 = CONCAT44(uVar9,uVar9);
  uVar10 = PackedFloatingMUL(uVar5,uVar7);
  uVar5 = CONCAT44((int)uVar7,(int)uVar6);
  uVar11 = PackedFloatingMUL(uVar7,uVar13);
  uVar8 = PackedFloatingMUL(CONCAT44((int)uVar8,(int)uVar8),uVar5);
  uVar7 = PackedFloatingMUL(uVar5,CONCAT44(uVar1,uVar1));
  uVar5 = PackedFloatingMUL(uVar5,uVar13);
  uVar6 = CONCAT44((int)((ulonglong)uVar8 >> 0x20),(int)((ulonglong)uVar7 >> 0x20));
  param_1[6] = 0;
  uVar13 = PackedFloatingADD(CONCAT44((int)uVar8,(int)uVar8),uVar10);
  uVar8 = PackedFloatingAccumulate(uVar10,uVar10);
  uVar5 = CONCAT44((int)((ulonglong)uVar11 >> 0x20),(int)uVar5);
  uVar2 = PackedFloatingSUBR(uVar8,CONCAT44(DAT_004dbc64,DAT_004dbc60));
  uVar10 = PackedFloatingSUBR(uVar13,CONCAT44(DAT_004dbc64,DAT_004dbc60));
  uVar8 = PackedFloatingADD(uVar6,uVar5);
  param_1[5] = uVar2 & 0xffffffff;
  uVar3 = PackedFloatingSUB(uVar6,uVar5);
  uVar2 = PackedFloatingADD(uVar7,uVar11);
  uVar5 = PackedFloatingSUB(uVar7,uVar11);
  param_1[3] = uVar2 & 0xffffffff;
  uVar2 = (ulonglong)DAT_004dbc60;
  param_1[1] = uVar3 >> 0x20;
  param_1[2] = CONCAT44((int)uVar10,(int)uVar3);
  *param_1 = CONCAT44((int)uVar8,(int)((ulonglong)uVar10 >> 0x20));
  param_1[4] = CONCAT44((int)uVar5,(int)((ulonglong)uVar8 >> 0x20));
  param_1[7] = uVar2 << 0x20;
  FastExitMediaState();
  return;
}

