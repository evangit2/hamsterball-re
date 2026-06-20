
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void SSE2_QuaternionSLERP(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3,uint param_4)

{
  undefined4 uVar1;
  uint uVar2;
  undefined8 uVar4;
  ulonglong uVar5;
  undefined8 uVar6;
  ulonglong uVar7;
  undefined8 uVar8;
  undefined8 uVar9;
  ulonglong uVar10;
  ulonglong uVar3;
  
  FastExitMediaState();
  uVar4 = PackedFloatingMUL(*param_2,*param_3);
  uVar6 = PackedFloatingMUL(param_2[1],param_3[1]);
  uVar4 = PackedFloatingADD(uVar4,uVar6);
  uVar3 = PackedFloatingSUB((ulonglong)DAT_004dbce8,(ulonglong)param_4);
  uVar1 = (undefined4)uVar3;
  uVar5 = PackedFloatingAccumulate(uVar4,uVar4);
  uVar10 = PackedFloatingCompareGE(0,uVar5);
  uVar10 = uVar10 & _DAT_004dbd30;
  uVar5 = uVar5 ^ uVar10;
  uVar6 = PackedFloatingCompareGE(DAT_004dbd48,uVar5);
  uVar4 = PackedFloatingMUL(uVar5,uVar5);
  uVar4 = PackedFloatingSUBR(uVar4,CONCAT44(_UNK_004dbcec,DAT_004dbce8));
  if ((int)uVar6 != 0) {
    uVar8 = PackedFloatingReciprocalSQRAprox((ulonglong)param_4,uVar4);
    uVar6 = PackedFloatingMUL(uVar8,uVar8);
    uVar4 = PackedFloatingReciprocalSQRIter1(uVar4,uVar6);
    uVar7 = PackedFloatingReciprocalIter2(uVar4,uVar8);
    uVar4 = FloatingReciprocalAprox(uVar6,uVar7);
    uVar6 = PackedFloatingReciprocalIter1(uVar7,uVar4);
    PackedFloatingReciprocalIter2(uVar6,uVar4);
    uVar5 = SIMD_PackedExp();
    PackedFloatingMUL(uVar5,(ulonglong)param_4);
    uVar2 = SSE2_Tan();
    PackedFloatingMUL(uVar5 & 0xffffffff,uVar3 & 0xffffffff);
    uVar4 = SSE2_Tan();
    uVar10 = uVar10 & 0xffffffff;
    uVar4 = PackedFloatingMUL(uVar4,uVar7 & 0xffffffff);
    uVar1 = (undefined4)uVar4;
    uVar4 = PackedFloatingMUL((ulonglong)uVar2,uVar7 & 0xffffffff);
    param_4 = (uint)uVar4;
  }
  uVar2 = param_4 ^ (uint)uVar10;
  uVar6 = CONCAT44(uVar2,uVar2);
  uVar4 = PackedFloatingMUL(CONCAT44(uVar1,uVar1),*param_2);
  uVar8 = PackedFloatingMUL(uVar6,*param_3);
  uVar9 = PackedFloatingMUL(CONCAT44(uVar1,uVar1),param_2[1]);
  uVar6 = PackedFloatingMUL(uVar6,param_3[1]);
  uVar4 = PackedFloatingADD(uVar4,uVar8);
  uVar6 = PackedFloatingADD(uVar9,uVar6);
  *param_1 = uVar4;
  param_1[1] = uVar6;
  FastExitMediaState();
  return;
}

