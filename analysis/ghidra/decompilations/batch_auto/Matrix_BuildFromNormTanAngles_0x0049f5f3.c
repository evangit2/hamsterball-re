
void Matrix_BuildFromNormTanAngles(undefined8 *param_1,ulonglong *param_2)

{
  ulonglong uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  ulonglong uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  undefined4 uVar9;
  ulonglong local_20;
  uint local_18;
  
  local_20 = *param_2;
  local_18 = (uint)param_2[1];
  Vec2_NormalizeSSE2(&local_20,&local_20);
  uVar1 = __libm_sse2_tan();
  uVar2 = PackedFloatingSUBR(uVar1,CONCAT44(DAT_004dbc64,DAT_004dbc60));
  uVar9 = (undefined4)(uVar1 >> 0x20);
  uVar4 = (ulonglong)local_18;
  uVar5 = PackedFloatingMUL(CONCAT44(local_18,local_18),local_20);
  uVar2 = CONCAT44((int)uVar2,(int)uVar2);
  uVar3 = PackedFloatingMUL(CONCAT44(local_18,(int)(local_20 >> 0x20)),local_20);
  uVar8 = CONCAT44((int)uVar1,(int)uVar1);
  uVar3 = PackedFloatingMUL(uVar3,uVar2);
  uVar5 = PackedFloatingMUL(uVar5,uVar2);
  uVar6 = PackedFloatingMUL(local_20,local_20);
  uVar7 = PackedFloatingMUL(uVar4,uVar4);
  uVar6 = PackedFloatingMUL(uVar6,uVar2);
  uVar2 = PackedFloatingMUL(uVar7,uVar2);
  uVar6 = PackedFloatingADD(uVar6,uVar8);
  uVar1 = PackedFloatingADD(uVar2,uVar8);
  uVar2 = CONCAT44(uVar9,uVar9);
  uVar8 = PackedFloatingMUL(local_20,uVar2);
  uVar2 = PackedFloatingMUL(uVar4,uVar2);
  param_1[6] = 0;
  uVar7 = CONCAT44((int)uVar8,(int)uVar2);
  uVar9 = (undefined4)((ulonglong)uVar8 >> 0x20);
  uVar8 = CONCAT44(uVar9,uVar9);
  param_1[5] = uVar1 & 0xffffffff;
  uVar2 = PackedFloatingSUB(uVar3,uVar7);
  uVar4 = PackedFloatingADD(uVar3,uVar7);
  uVar1 = PackedFloatingSUB(uVar5,uVar8);
  *param_1 = CONCAT44((int)uVar4,(int)uVar6);
  param_1[1] = uVar1 & 0xffffffff;
  uVar1 = (ulonglong)DAT_004dbc64;
  uVar8 = PackedFloatingADD(CONCAT44((int)uVar5,(int)uVar5),uVar8);
  param_1[3] = uVar4 >> 0x20;
  param_1[7] = uVar1 << 0x20;
  param_1[4] = CONCAT44((int)((ulonglong)uVar2 >> 0x20),(int)((ulonglong)uVar8 >> 0x20));
  param_1[2] = CONCAT44((int)((ulonglong)uVar6 >> 0x20),(int)uVar2);
  FastExitMediaState();
  return;
}

