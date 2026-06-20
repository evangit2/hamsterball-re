
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void Matrix_RotateEllipseIntersectionSSE
               (undefined8 *param_1,undefined8 *param_2,ulonglong *param_3,ulonglong *param_4,
               ulonglong *param_5,ulonglong *param_6,ulonglong *param_7)

{
  undefined4 unaff_ESI;
  undefined8 uVar1;
  ulonglong uVar2;
  ulonglong uVar3;
  undefined8 uVar4;
  undefined4 uVar5;
  undefined8 uVar6;
  ulonglong uVar7;
  ulonglong uVar8;
  undefined8 uVar9;
  ulonglong uVar10;
  undefined8 uVar11;
  undefined8 uVar12;
  ulonglong uVar13;
  undefined8 uVar14;
  ulonglong local_a8;
  ulonglong local_a0;
  ulonglong local_88;
  ulonglong local_80;
  ulonglong local_78;
  ulonglong local_70;
  undefined4 local_20;
  
  local_a8 = *param_4;
  local_a0 = param_4[1];
  uVar8 = *param_5;
  uVar13 = param_5[1];
  uVar1 = PackedFloatingADD(local_a8,uVar8);
  uVar4 = PackedFloatingADD(local_a0,uVar13);
  uVar6 = PackedFloatingSUB(local_a8,uVar8);
  uVar1 = PackedFloatingMUL(uVar1,uVar1);
  uVar9 = PackedFloatingSUB(local_a0,uVar13);
  uVar4 = PackedFloatingMUL(uVar4,uVar4);
  uVar6 = PackedFloatingMUL(uVar6,uVar6);
  uVar9 = PackedFloatingMUL(uVar9,uVar9);
  uVar1 = PackedFloatingADD(uVar1,uVar4);
  uVar6 = PackedFloatingADD(uVar6,uVar9);
  local_88 = *param_6;
  local_80 = param_6[1];
  uVar11 = PackedFloatingADD(uVar8,local_88);
  uVar12 = PackedFloatingADD(uVar13,local_80);
  uVar4 = PackedFloatingSUB(uVar8,local_88);
  uVar9 = PackedFloatingSUB(uVar13,local_80);
  uVar11 = PackedFloatingMUL(uVar11,uVar11);
  uVar12 = PackedFloatingMUL(uVar12,uVar12);
  uVar4 = PackedFloatingMUL(uVar4,uVar4);
  uVar9 = PackedFloatingMUL(uVar9,uVar9);
  uVar11 = PackedFloatingADD(uVar11,uVar12);
  uVar4 = PackedFloatingADD(uVar4,uVar9);
  uVar1 = PackedFloatingAccumulate(uVar1,uVar11);
  uVar4 = PackedFloatingAccumulate(uVar6,uVar4);
  uVar1 = PackedFloatingCompareGT(uVar4,uVar1);
  uVar7 = packsswb(uVar1,uVar1);
  if ((uVar7 & 1) != 0) {
    local_a8 = DAT_004dbc88 ^ *param_4;
    local_a0 = DAT_004dbc88 ^ param_4[1];
  }
  if ((uVar7 & 0x10000) != 0) {
    local_88 = DAT_004dbc88 ^ *param_6;
    local_80 = DAT_004dbc88 ^ param_6[1];
  }
  local_78 = *param_7;
  uVar7 = param_7[1];
  uVar6 = PackedFloatingADD(local_88,local_78);
  uVar9 = PackedFloatingADD(local_80,uVar7);
  uVar1 = PackedFloatingSUB(local_88,local_78);
  uVar4 = PackedFloatingSUB(local_80,uVar7);
  uVar6 = PackedFloatingMUL(uVar6,uVar6);
  uVar9 = PackedFloatingMUL(uVar9,uVar9);
  uVar1 = PackedFloatingMUL(uVar1,uVar1);
  uVar4 = PackedFloatingMUL(uVar4,uVar4);
  uVar6 = PackedFloatingADD(uVar6,uVar9);
  uVar1 = PackedFloatingADD(uVar1,uVar4);
  uVar4 = PackedFloatingAccumulate(uVar6,uVar6);
  uVar1 = PackedFloatingAccumulate(uVar1,uVar1);
  uVar10 = PackedFloatingCompareGT(uVar1,uVar4);
  local_70 = uVar7;
  if ((uVar10 & 1) != 0) {
    local_78 = DAT_004dbc88 ^ *param_7;
    local_70 = DAT_004dbc88 ^ param_7[1];
  }
  uVar1 = PackedFloatingMUL(uVar8,uVar8);
  uVar4 = PackedFloatingMUL(uVar13,uVar13);
  uVar1 = PackedFloatingADD(uVar1,uVar4);
  uVar1 = PackedFloatingAccumulate(uVar1,uVar1);
  uVar4 = FloatingReciprocalAprox(uVar7,uVar1);
  uVar7 = PackedFloatingCompareGT(uVar1,DAT_004dbcd0);
  uVar1 = PackedFloatingReciprocalIter1(uVar1,uVar4);
  uVar1 = PackedFloatingReciprocalIter2(uVar1,uVar4);
  uVar4 = PackedFloatingMUL((uVar8 ^ _DAT_004dbd30) & uVar7,uVar1);
  uVar11 = PackedFloatingMUL((uVar13 ^ _DAT_004dbc78) & uVar7,uVar1);
  uVar10 = CONCAT44((int)local_a8,(int)(local_a8 >> 0x20));
  uVar2 = CONCAT44((int)local_a0,(int)(local_a0 >> 0x20));
  uVar7 = PackedFloatingMUL(local_a8,uVar11);
  uVar1 = PackedFloatingMUL(local_a0,uVar4);
  uVar9 = PackedFloatingMUL(uVar2 ^ _DAT_004dbc78,uVar4);
  uVar6 = PackedFloatingMUL(uVar10,uVar11);
  uVar1 = PackedFloatingADD(uVar7 ^ _DAT_004dbc78,uVar1);
  uVar6 = PackedFloatingSUB(uVar6,uVar9);
  uVar14 = PackedFloatingMUL(uVar2,uVar11);
  uVar6 = PackedFloatingAccumulate(uVar6,uVar1);
  uVar12 = PackedFloatingMUL(uVar10 ^ _DAT_004dbc78,uVar4);
  uVar1 = PackedFloatingMUL(local_a8,uVar4);
  uVar9 = PackedFloatingMUL(local_a0 ^ _DAT_004dbc78,uVar11);
  uVar12 = PackedFloatingADD(uVar12,uVar14);
  uVar1 = PackedFloatingSUB(uVar9,uVar1);
  uVar10 = PackedFloatingAccumulate(uVar12,uVar1);
  uVar2 = CONCAT44((int)local_88,(int)(local_88 >> 0x20));
  uVar3 = CONCAT44((int)local_80,(int)(local_80 >> 0x20));
  uVar7 = PackedFloatingMUL(local_88,uVar11);
  uVar1 = PackedFloatingMUL(local_80,uVar4);
  uVar12 = PackedFloatingMUL(uVar3 ^ _DAT_004dbc78,uVar4);
  uVar9 = PackedFloatingMUL(uVar2,uVar11);
  uVar1 = PackedFloatingADD(uVar7 ^ _DAT_004dbc78,uVar1);
  uVar9 = PackedFloatingSUB(uVar9,uVar12);
  uVar14 = PackedFloatingMUL(uVar3,uVar11);
  uVar9 = PackedFloatingAccumulate(uVar9,uVar1);
  uVar12 = PackedFloatingMUL(uVar2 ^ _DAT_004dbc78,uVar4);
  uVar1 = PackedFloatingMUL(local_88,uVar4);
  uVar4 = PackedFloatingMUL(local_80 ^ _DAT_004dbc78,uVar11);
  uVar11 = PackedFloatingADD(uVar12,uVar14);
  uVar1 = PackedFloatingSUB(uVar4,uVar1);
  uVar7 = PackedFloatingAccumulate(uVar11,uVar1);
  uVar5 = (undefined4)(uVar10 >> 0x20);
  uVar4 = PackedFloatingCompareGE(CONCAT44(uVar5,uVar5),_DAT_004dbce8);
  uVar1 = uVar9;
  if ((int)uVar4 == 0) {
    uVar2 = SIMD_PackedPow2();
    uVar3 = SSE2_Tan();
    uVar4 = PackedFloatingCompareGE(uVar3 & _DAT_004dbca0,DAT_004dbca8);
    if ((int)uVar4 != 0) {
      uVar1 = FloatingReciprocalAprox(uVar1,uVar3);
      uVar4 = PackedFloatingReciprocalIter1(uVar3,uVar1);
      uVar4 = PackedFloatingReciprocalIter2(uVar4,uVar1);
      uVar4 = PackedFloatingMUL(CONCAT44((int)uVar2,(int)uVar2),uVar4);
      uVar6 = PackedFloatingMUL(uVar6,uVar4);
      uVar10 = PackedFloatingMUL(uVar10,uVar4);
    }
  }
  uVar10 = uVar10 & _DAT_004dbcb0;
  uVar5 = (undefined4)(uVar7 >> 0x20);
  uVar4 = PackedFloatingCompareGE(CONCAT44(uVar5,uVar5),_DAT_004dbce8);
  if ((int)uVar4 == 0) {
    uVar2 = SIMD_PackedPow2();
    uVar3 = SSE2_Tan();
    uVar4 = PackedFloatingCompareGE(uVar3 & _DAT_004dbca0,DAT_004dbca8);
    if ((int)uVar4 != 0) {
      uVar4 = FloatingReciprocalAprox(uVar1,uVar3);
      uVar1 = PackedFloatingReciprocalIter1(uVar3,uVar4);
      uVar1 = PackedFloatingReciprocalIter2(uVar1,uVar4);
      uVar1 = PackedFloatingMUL(CONCAT44((int)uVar2,(int)uVar2),uVar1);
      uVar9 = PackedFloatingMUL(uVar9,uVar1);
      uVar7 = PackedFloatingMUL(uVar7,uVar1);
    }
  }
  uVar1 = PackedFloatingADD(uVar6,uVar9);
  uVar4 = PackedFloatingADD(uVar10,uVar7 & _DAT_004dbcb0);
  uVar1 = PackedFloatingMUL(uVar1,_DAT_004e42c0);
  uVar2 = PackedFloatingMUL(uVar4,_DAT_004e42c0);
  uVar4 = PackedFloatingMUL(uVar1,uVar1);
  uVar6 = PackedFloatingMUL(uVar2 & 0xffffffff,uVar2 & 0xffffffff);
  uVar4 = PackedFloatingADD(uVar4,uVar6);
  uVar4 = PackedFloatingAccumulate(uVar4,uVar4);
  uVar6 = PackedFloatingReciprocalSQRAprox(uVar6,uVar4);
  uVar9 = PackedFloatingMUL(uVar6,uVar6);
  uVar9 = PackedFloatingReciprocalSQRIter1(uVar9,uVar4);
  uVar6 = PackedFloatingReciprocalIter2(uVar9,uVar6);
  PackedFloatingMUL(uVar4,uVar6);
  uVar7 = __libm_sse2_tan();
  uVar10 = PackedFloatingCompareGE(uVar7 & _DAT_004dbc98,DAT_004dbca8);
  uVar5 = (undefined4)(uVar7 >> 0x20);
  uVar4 = PackedFloatingMUL(CONCAT44(uVar5,uVar5),CONCAT44(unaff_ESI,(int)uVar6));
  local_20 = (undefined4)uVar2;
  if ((uVar10 & 0x100000000) != 0) {
    uVar4 = CONCAT44((int)uVar4,(int)uVar4);
    uVar6 = PackedFloatingMUL(uVar2 & 0xffffffff,uVar4);
    local_20 = (undefined4)uVar6;
    uVar1 = PackedFloatingMUL(uVar1,uVar4);
  }
  uVar10 = CONCAT44((int)uVar7,local_20);
  uVar2 = CONCAT44((int)uVar1,(int)((ulonglong)uVar1 >> 0x20));
  uVar3 = CONCAT44(local_20,(int)uVar7);
  uVar7 = PackedFloatingMUL(uVar1,uVar13);
  uVar4 = PackedFloatingMUL(uVar10,uVar8);
  uVar9 = PackedFloatingMUL(uVar3 ^ _DAT_004dbc78,uVar8);
  uVar6 = PackedFloatingMUL(uVar2,uVar13);
  uVar4 = PackedFloatingADD(uVar7 ^ _DAT_004dbc78,uVar4);
  uVar6 = PackedFloatingSUB(uVar6,uVar9);
  uVar11 = PackedFloatingMUL(uVar3,uVar13);
  uVar6 = PackedFloatingAccumulate(uVar6,uVar4);
  uVar9 = PackedFloatingMUL(uVar2 ^ _DAT_004dbc78,uVar8);
  uVar10 = uVar10 ^ _DAT_004dbc78;
  uVar1 = PackedFloatingMUL(uVar1,uVar8);
  *param_1 = uVar6;
  uVar4 = PackedFloatingMUL(uVar10,uVar13);
  uVar9 = PackedFloatingADD(uVar9,uVar11);
  uVar1 = PackedFloatingSUB(uVar4,uVar1);
  uVar1 = PackedFloatingAccumulate(uVar9,uVar1);
  param_1[1] = uVar1;
  uVar1 = PackedFloatingMUL(local_88,local_88);
  uVar4 = PackedFloatingMUL(local_80,local_80);
  uVar1 = PackedFloatingADD(uVar1,uVar4);
  uVar1 = PackedFloatingAccumulate(uVar1,uVar1);
  uVar4 = FloatingReciprocalAprox(uVar6,uVar1);
  uVar7 = PackedFloatingCompareGT(uVar1,DAT_004dbcd0);
  uVar1 = PackedFloatingReciprocalIter1(uVar1,uVar4);
  uVar1 = PackedFloatingReciprocalIter2(uVar1,uVar4);
  uVar4 = PackedFloatingMUL((local_88 ^ _DAT_004dbd30) & uVar7,uVar1);
  uVar11 = PackedFloatingMUL((local_80 ^ _DAT_004dbc78) & uVar7,uVar1);
  uVar10 = CONCAT44((int)uVar8,(int)(uVar8 >> 0x20));
  uVar2 = CONCAT44((int)uVar13,(int)(uVar13 >> 0x20));
  uVar7 = PackedFloatingMUL(uVar8,uVar11);
  uVar1 = PackedFloatingMUL(uVar13,uVar4);
  uVar9 = PackedFloatingMUL(uVar2 ^ _DAT_004dbc78,uVar4);
  uVar6 = PackedFloatingMUL(uVar10,uVar11);
  uVar1 = PackedFloatingADD(uVar7 ^ _DAT_004dbc78,uVar1);
  uVar6 = PackedFloatingSUB(uVar6,uVar9);
  uVar14 = PackedFloatingMUL(uVar2,uVar11);
  uVar6 = PackedFloatingAccumulate(uVar6,uVar1);
  uVar12 = PackedFloatingMUL(uVar10 ^ _DAT_004dbc78,uVar4);
  uVar1 = PackedFloatingMUL(uVar8,uVar4);
  uVar9 = PackedFloatingMUL(uVar13 ^ _DAT_004dbc78,uVar11);
  uVar12 = PackedFloatingADD(uVar12,uVar14);
  uVar1 = PackedFloatingSUB(uVar9,uVar1);
  uVar13 = PackedFloatingAccumulate(uVar12,uVar1);
  uVar7 = CONCAT44((int)local_78,(int)(local_78 >> 0x20));
  uVar10 = CONCAT44((int)local_70,(int)(local_70 >> 0x20));
  uVar8 = PackedFloatingMUL(local_78,uVar11);
  uVar1 = PackedFloatingMUL(local_70,uVar4);
  uVar12 = PackedFloatingMUL(uVar10 ^ _DAT_004dbc78,uVar4);
  uVar9 = PackedFloatingMUL(uVar7,uVar11);
  uVar1 = PackedFloatingADD(uVar8 ^ _DAT_004dbc78,uVar1);
  uVar9 = PackedFloatingSUB(uVar9,uVar12);
  uVar14 = PackedFloatingMUL(uVar10,uVar11);
  uVar9 = PackedFloatingAccumulate(uVar9,uVar1);
  uVar12 = PackedFloatingMUL(uVar7 ^ _DAT_004dbc78,uVar4);
  uVar1 = PackedFloatingMUL(local_78,uVar4);
  uVar4 = PackedFloatingMUL(local_70 ^ _DAT_004dbc78,uVar11);
  uVar11 = PackedFloatingADD(uVar12,uVar14);
  uVar1 = PackedFloatingSUB(uVar4,uVar1);
  uVar8 = PackedFloatingAccumulate(uVar11,uVar1);
  uVar5 = (undefined4)(uVar13 >> 0x20);
  uVar4 = PackedFloatingCompareGE(CONCAT44(uVar5,uVar5),_DAT_004dbce8);
  uVar1 = uVar9;
  if ((int)uVar4 == 0) {
    uVar7 = SIMD_PackedPow2();
    uVar10 = SSE2_Tan();
    uVar4 = PackedFloatingCompareGE(uVar10 & _DAT_004dbca0,DAT_004dbca8);
    if ((int)uVar4 != 0) {
      uVar1 = FloatingReciprocalAprox(uVar1,uVar10);
      uVar4 = PackedFloatingReciprocalIter1(uVar10,uVar1);
      uVar4 = PackedFloatingReciprocalIter2(uVar4,uVar1);
      uVar4 = PackedFloatingMUL(CONCAT44((int)uVar7,(int)uVar7),uVar4);
      uVar6 = PackedFloatingMUL(uVar6,uVar4);
      uVar13 = PackedFloatingMUL(uVar13,uVar4);
    }
  }
  uVar13 = uVar13 & _DAT_004dbcb0;
  uVar5 = (undefined4)(uVar8 >> 0x20);
  uVar4 = PackedFloatingCompareGE(CONCAT44(uVar5,uVar5),_DAT_004dbce8);
  if ((int)uVar4 == 0) {
    uVar7 = SIMD_PackedPow2();
    uVar10 = SSE2_Tan();
    uVar4 = PackedFloatingCompareGE(uVar10 & _DAT_004dbca0,DAT_004dbca8);
    if ((int)uVar4 != 0) {
      uVar4 = FloatingReciprocalAprox(uVar1,uVar10);
      uVar1 = PackedFloatingReciprocalIter1(uVar10,uVar4);
      uVar1 = PackedFloatingReciprocalIter2(uVar1,uVar4);
      uVar1 = PackedFloatingMUL(CONCAT44((int)uVar7,(int)uVar7),uVar1);
      uVar9 = PackedFloatingMUL(uVar9,uVar1);
      uVar8 = PackedFloatingMUL(uVar8,uVar1);
    }
  }
  uVar1 = PackedFloatingADD(uVar6,uVar9);
  uVar4 = PackedFloatingADD(uVar13,uVar8 & _DAT_004dbcb0);
  uVar1 = PackedFloatingMUL(uVar1,_DAT_004e42c0);
  uVar7 = PackedFloatingMUL(uVar4,_DAT_004e42c0);
  uVar4 = PackedFloatingMUL(uVar1,uVar1);
  uVar6 = PackedFloatingMUL(uVar7 & 0xffffffff,uVar7 & 0xffffffff);
  uVar4 = PackedFloatingADD(uVar4,uVar6);
  uVar4 = PackedFloatingAccumulate(uVar4,uVar4);
  uVar6 = PackedFloatingReciprocalSQRAprox(uVar6,uVar4);
  uVar9 = PackedFloatingMUL(uVar6,uVar6);
  uVar9 = PackedFloatingReciprocalSQRIter1(uVar9,uVar4);
  uVar6 = PackedFloatingReciprocalIter2(uVar9,uVar6);
  PackedFloatingMUL(uVar4,uVar6);
  uVar8 = __libm_sse2_tan();
  uVar13 = PackedFloatingCompareGE(uVar8 & _DAT_004dbc98,DAT_004dbca8);
  uVar5 = (undefined4)(uVar8 >> 0x20);
  uVar4 = PackedFloatingMUL(CONCAT44(uVar5,uVar5),CONCAT44(unaff_ESI,(int)uVar6));
  local_20 = (undefined4)uVar7;
  if ((uVar13 & 0x100000000) != 0) {
    uVar4 = CONCAT44((int)uVar4,(int)uVar4);
    uVar6 = PackedFloatingMUL(uVar7 & 0xffffffff,uVar4);
    local_20 = (undefined4)uVar6;
    uVar1 = PackedFloatingMUL(uVar1,uVar4);
  }
  uVar13 = CONCAT44((int)uVar8,local_20);
  uVar7 = CONCAT44((int)uVar1,(int)((ulonglong)uVar1 >> 0x20));
  uVar10 = CONCAT44(local_20,(int)uVar8);
  uVar8 = PackedFloatingMUL(uVar1,local_80);
  uVar4 = PackedFloatingMUL(uVar13,local_88);
  uVar9 = PackedFloatingMUL(uVar10 ^ _DAT_004dbc78,local_88);
  uVar6 = PackedFloatingMUL(uVar7,local_80);
  uVar4 = PackedFloatingADD(uVar8 ^ _DAT_004dbc78,uVar4);
  uVar6 = PackedFloatingSUB(uVar6,uVar9);
  uVar9 = PackedFloatingMUL(uVar10,local_80);
  uVar4 = PackedFloatingAccumulate(uVar6,uVar4);
  uVar6 = PackedFloatingMUL(uVar7 ^ _DAT_004dbc78,local_88);
  uVar13 = uVar13 ^ _DAT_004dbc78;
  uVar1 = PackedFloatingMUL(uVar1,local_88);
  *param_2 = uVar4;
  uVar4 = PackedFloatingMUL(uVar13,local_80);
  uVar6 = PackedFloatingADD(uVar6,uVar9);
  uVar1 = PackedFloatingSUB(uVar4,uVar1);
  uVar1 = PackedFloatingAccumulate(uVar6,uVar1);
  param_2[1] = uVar1;
  *param_3 = local_88;
  param_3[1] = local_80;
  FastExitMediaState();
  return;
}

