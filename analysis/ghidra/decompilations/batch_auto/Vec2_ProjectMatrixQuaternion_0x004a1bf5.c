
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ulonglong *
Vec2_ProjectMatrixQuaternion
          (ulonglong *param_1,ulonglong *param_2,undefined8 *param_3,undefined8 *param_4,
          undefined8 *param_5,undefined8 *param_6)

{
  ulonglong uVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined4 uVar6;
  undefined8 uVar4;
  undefined8 uVar5;
  undefined8 in_MM5;
  undefined8 uVar7;
  undefined8 in_MM6;
  undefined8 local_44 [8];
  
  switch(((param_6 != (undefined8 *)0x0) << 1 | param_5 != (undefined8 *)0x0) << 1 |
         param_4 != (undefined8 *)0x0) {
  case '\0':
    Matrix_IdentitySSE(local_44);
    goto switchD_004a1c2b_default;
  case '\x01':
    param_6 = param_4;
    goto LAB_004a1c6f;
  case '\x02':
    param_6 = param_5;
    goto LAB_004a1c6f;
  case '\x03':
    param_6 = param_5;
    break;
  case '\x04':
    goto LAB_004a1c6f;
  case '\x05':
    break;
  case '\x06':
    param_4 = param_5;
    break;
  case '\a':
    Matrix4x4_Multiply_SSE2(local_44,param_6,param_5);
    param_6 = local_44;
    break;
  default:
    goto switchD_004a1c2b_default;
  }
  Matrix4x4_Multiply_SSE2(local_44,param_6,param_4);
  param_6 = local_44;
LAB_004a1c6f:
  Matrix_BuildFromQuaternion3x3(local_44,(undefined4 *)0x0,param_6);
switchD_004a1c2b_default:
  if (param_3 != (undefined8 *)0x0) {
    FastExitMediaState();
    uVar2 = PackedIntToFloatingDwordConv(*param_3,*param_3);
    uVar5 = param_3[2];
    uVar3 = PackedIntToFloatingDwordConv(param_3[1],param_3[1]);
    uVar6 = (undefined4)((ulonglong)uVar3 >> 0x20);
    uVar4 = CONCAT44(uVar6,uVar6);
    uVar7 = FloatingReciprocalAprox(in_MM5,uVar3);
    uVar4 = FloatingReciprocalAprox(uVar4,uVar4);
    uVar2 = PackedFloatingSUB(*param_2,uVar2);
    uVar4 = CONCAT44((int)uVar4,(int)uVar7);
    uVar3 = PackedFloatingReciprocalIter1(uVar3,uVar4);
    uVar3 = PackedFloatingReciprocalIter2(uVar3,uVar4);
    uVar3 = PackedFloatingMUL(uVar3,DAT_004dbcf8);
    uVar2 = PackedFloatingMUL(uVar2,uVar3);
    uVar6 = (undefined4)((ulonglong)uVar5 >> 0x20);
    uVar1 = PackedFloatingSUB(uVar2,_DAT_004dbce8);
    uVar2 = PackedFloatingSUB(CONCAT44(uVar6,uVar6),uVar5);
    uVar3 = FloatingReciprocalAprox(in_MM6,uVar2);
    uVar2 = PackedFloatingReciprocalIter1(uVar2,uVar3);
    uVar2 = PackedFloatingReciprocalIter2(uVar2,uVar3);
    uVar5 = PackedFloatingSUB((ulonglong)(uint)param_2[1],uVar5);
    uVar5 = PackedFloatingMUL(uVar5,uVar2);
    *param_1 = uVar1 ^ DAT_004dbce0;
    *(int *)(param_1 + 1) = (int)uVar5;
    FastExitMediaState();
    param_2 = param_1;
  }
  Vec2_TransformMatrixPerspDivide(param_1,param_2,local_44);
  return param_1;
}

