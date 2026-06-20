
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ulonglong *
Vec2_ProjectMatrixInverse
          (ulonglong *param_1,ulonglong *param_2,undefined8 *param_3,undefined8 *param_4,
          undefined8 *param_5,undefined8 *param_6)

{
  undefined8 uVar1;
  undefined8 uVar2;
  ulonglong uVar3;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined4 uVar7;
  undefined8 uVar6;
  undefined8 uVar8;
  unkbyte10 in_ST5;
  undefined8 uVar9;
  unkbyte10 in_ST6;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  uVar2 = (undefined8)in_ST5;
  uVar9 = (undefined8)in_ST6;
  switch(((param_6 != (undefined8 *)0x0) << 1 | param_5 != (undefined8 *)0x0) << 1 |
         param_4 != (undefined8 *)0x0) {
  case '\0':
    local_c = 0;
    local_10 = 0;
    local_14 = 0;
    local_18 = 0;
    local_20 = 0;
    local_24 = 0;
    local_28 = 0;
    local_2c = 0;
    local_34 = 0;
    local_38 = 0;
    local_3c = 0;
    local_40 = 0;
    local_8 = 0x3f800000;
    local_1c = 0x3f800000;
    local_30 = 0x3f800000;
    local_44 = 0x3f800000;
    break;
  case '\x01':
    FastExitMediaState();
    Matrix_Inverse4x4_SSE2((undefined8 *)&local_44,(undefined4 *)0x0,param_4);
    uVar2 = (undefined8)in_ST5;
    uVar9 = (undefined8)in_ST6;
    FastExitMediaState();
    break;
  case '\x02':
    FastExitMediaState();
    Matrix_Inverse4x4_SSE2((undefined8 *)&local_44,(undefined4 *)0x0,param_5);
    uVar2 = (undefined8)in_ST5;
    uVar9 = (undefined8)in_ST6;
    FastExitMediaState();
    break;
  case '\x03':
    FastExitMediaState();
    Matrix4x4_Multiply_SSE2((undefined8 *)&local_44,param_5,param_4);
    Matrix_Inverse4x4_SSE2((undefined8 *)&local_44,(undefined4 *)0x0,(undefined8 *)&local_44);
    uVar2 = (undefined8)in_ST5;
    uVar9 = (undefined8)in_ST6;
    FastExitMediaState();
    break;
  case '\x04':
    FastExitMediaState();
    Matrix_Inverse4x4_SSE2((undefined8 *)&local_44,(undefined4 *)0x0,param_6);
    uVar2 = (undefined8)in_ST5;
    uVar9 = (undefined8)in_ST6;
    FastExitMediaState();
    break;
  case '\x05':
    FastExitMediaState();
    Matrix4x4_Multiply_SSE2((undefined8 *)&local_44,param_6,param_4);
    Matrix_Inverse4x4_SSE2((undefined8 *)&local_44,(undefined4 *)0x0,(undefined8 *)&local_44);
    uVar2 = (undefined8)in_ST5;
    uVar9 = (undefined8)in_ST6;
    FastExitMediaState();
    break;
  case '\x06':
    FastExitMediaState();
    Matrix4x4_Multiply_SSE2((undefined8 *)&local_44,param_6,param_5);
    Matrix_Inverse4x4_SSE2((undefined8 *)&local_44,(undefined4 *)0x0,(undefined8 *)&local_44);
    uVar2 = (undefined8)in_ST5;
    uVar9 = (undefined8)in_ST6;
    FastExitMediaState();
    break;
  case '\a':
    FastExitMediaState();
    Matrix4x4_Multiply_SSE2((undefined8 *)&local_44,param_6,param_5);
    Matrix4x4_Multiply_SSE2((undefined8 *)&local_44,(undefined8 *)&local_44,param_4);
    Matrix_Inverse4x4_SSE2((undefined8 *)&local_44,(undefined4 *)0x0,(undefined8 *)&local_44);
    uVar2 = (undefined8)in_ST5;
    uVar9 = (undefined8)in_ST6;
    FastExitMediaState();
  }
  if (param_3 != (undefined8 *)0x0) {
    FastExitMediaState();
    uVar4 = PackedIntToFloatingDwordConv(*param_3,*param_3);
    uVar1 = param_3[2];
    uVar5 = PackedIntToFloatingDwordConv(param_3[1],param_3[1]);
    uVar7 = (undefined4)((ulonglong)uVar5 >> 0x20);
    uVar6 = CONCAT44(uVar7,uVar7);
    uVar8 = FloatingReciprocalAprox(uVar2,uVar5);
    uVar6 = FloatingReciprocalAprox(uVar6,uVar6);
    uVar2 = PackedFloatingSUB(*param_2,uVar4);
    uVar6 = CONCAT44((int)uVar6,(int)uVar8);
    uVar4 = PackedFloatingReciprocalIter1(uVar5,uVar6);
    uVar4 = PackedFloatingReciprocalIter2(uVar4,uVar6);
    uVar4 = PackedFloatingMUL(uVar4,DAT_004dbcf8);
    uVar2 = PackedFloatingMUL(uVar2,uVar4);
    uVar7 = (undefined4)((ulonglong)uVar1 >> 0x20);
    uVar3 = PackedFloatingSUB(uVar2,_DAT_004dbce8);
    uVar2 = PackedFloatingSUB(CONCAT44(uVar7,uVar7),uVar1);
    uVar9 = FloatingReciprocalAprox(uVar9,uVar2);
    uVar2 = PackedFloatingReciprocalIter1(uVar2,uVar9);
    uVar9 = PackedFloatingReciprocalIter2(uVar2,uVar9);
    uVar2 = PackedFloatingSUB((ulonglong)(uint)param_2[1],uVar1);
    uVar2 = PackedFloatingMUL(uVar2,uVar9);
    *param_1 = uVar3 ^ DAT_004dbce0;
    *(int *)(param_1 + 1) = (int)uVar2;
    FastExitMediaState();
    param_2 = param_1;
  }
  Vec2_TransformMatrixPerspDivide(param_1,param_2,(undefined8 *)&local_44);
  return param_1;
}

