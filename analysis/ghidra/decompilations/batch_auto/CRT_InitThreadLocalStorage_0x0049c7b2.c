
undefined8 *
CRT_InitThreadLocalStorage
          (undefined8 *param_1,undefined8 *param_2,undefined8 *param_3,undefined8 *param_4,
          uint param_5,uint param_6)

{
  ulonglong uVar1;
  undefined8 uVar2;
  undefined8 in_MM2;
  undefined8 uVar3;
  undefined8 local_28 [2];
  undefined8 local_18 [2];
  
  FastExitMediaState();
  uVar1 = PackedFloatingADD((ulonglong)param_5,(ulonglong)param_6);
  SSE2_QuaternionSLERP(local_28,param_2,param_3,(uint)uVar1);
  SSE2_QuaternionSLERP(local_18,param_2,param_4,(uint)uVar1);
  uVar3 = FloatingReciprocalAprox(in_MM2,uVar1 & 0xffffffff);
  uVar2 = PackedFloatingReciprocalIter1(uVar1 & 0xffffffff,uVar3);
  uVar2 = PackedFloatingReciprocalIter2(uVar2,uVar3);
  uVar2 = PackedFloatingMUL(uVar2,(ulonglong)param_6);
  SSE2_QuaternionSLERP(param_1,local_28,local_18,(uint)uVar2);
  FastExitMediaState();
  return param_1;
}

