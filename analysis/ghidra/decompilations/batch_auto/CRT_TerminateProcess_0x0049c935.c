
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void CRT_TerminateProcess(undefined8 *param_1,undefined8 *param_2)

{
  ulonglong uVar1;
  ulonglong uVar2;
  undefined8 uVar3;
  ulonglong uVar4;
  undefined4 uVar6;
  undefined8 uVar5;
  undefined8 in_MM4;
  undefined8 uVar7;
  
  FastExitMediaState();
  uVar3 = *param_2;
  uVar4 = param_2[1];
  uVar6 = (undefined4)(uVar4 >> 0x20);
  uVar5 = PackedFloatingCompareGE(CONCAT44(uVar6,uVar6),_DAT_004dbce8);
  if ((int)uVar5 == 0) {
    uVar1 = SIMD_PackedPow2();
    uVar2 = SSE2_Tan();
    uVar5 = PackedFloatingCompareGE(uVar2 & _DAT_004dbca0,DAT_004dbca8);
    uVar3 = *param_2;
    uVar4 = param_2[1];
    if ((int)uVar5 != 0) {
      uVar7 = FloatingReciprocalAprox(in_MM4,uVar2);
      uVar5 = PackedFloatingReciprocalIter1(uVar2,uVar7);
      uVar5 = PackedFloatingReciprocalIter2(uVar5,uVar7);
      uVar5 = PackedFloatingMUL(CONCAT44((int)uVar1,(int)uVar1),uVar5);
      uVar3 = PackedFloatingMUL(uVar3,uVar5);
      uVar4 = PackedFloatingMUL(uVar4,uVar5);
    }
  }
  uVar4 = uVar4 & _DAT_004dbcb0;
  *param_1 = uVar3;
  param_1[1] = uVar4;
  FastExitMediaState();
  return;
}

