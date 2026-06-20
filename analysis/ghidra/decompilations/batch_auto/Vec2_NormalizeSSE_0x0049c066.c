
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void Vec2_NormalizeSSE(undefined8 *param_1,undefined8 *param_2,undefined4 *param_3)

{
  undefined8 uVar1;
  ulonglong uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  
  uVar4 = _DAT_004dbc50;
  uVar5 = *param_1;
  uVar1 = param_1[1];
  uVar6 = PackedFloatingMUL(uVar5,uVar5);
  uVar3 = PackedFloatingMUL(uVar1,uVar1);
  uVar6 = PackedFloatingADD(uVar6,uVar3);
  uVar6 = PackedFloatingAccumulate(uVar6,uVar6);
  uVar7 = PackedFloatingCompareGE(DAT_004dbcd0,uVar6);
  uVar3 = PackedFloatingReciprocalSQRAprox(uVar3,uVar6);
  if ((int)uVar7 == 0) {
    uVar4 = PackedFloatingMUL(uVar3,uVar3);
    if (param_2 != (undefined8 *)0x0) {
      uVar4 = PackedFloatingReciprocalSQRIter1(uVar4,uVar6);
      uVar4 = PackedFloatingReciprocalIter2(uVar4,uVar3);
      uVar3 = PackedFloatingMUL(uVar5,uVar4);
      uVar5 = PackedFloatingMUL(uVar4,uVar1);
      *param_2 = uVar3;
      *(int *)(param_2 + 1) = (int)uVar5;
    }
    if (param_3 != (undefined4 *)0x0) {
      uVar2 = SIMD_PackedPow2();
      uVar5 = PackedFloatingMUL(uVar2,_DAT_004dbc68);
      *param_3 = (int)uVar5;
    }
  }
  else {
    if (param_2 != (undefined8 *)0x0) {
      *(undefined8 *)((int)param_2 + 4) = 0;
      *(int *)param_2 = (int)uVar4;
    }
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = 0;
    }
  }
  FastExitMediaState();
  return;
}

