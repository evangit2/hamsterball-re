/*
 * Function: Ball_SetTrajectory
 * Address: 0x00403850
 * Signature: Ball_SetTrajectory(...)
 *
 * Patterns: SEH frame, matrix math, ball. Calls: Ball_SetTrajectory, Matrix_Identity. Offsets: 9, Lines: 52
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
Ball_SetTrajectory(void *this,undefined4 param_1,float param_2,float param_3,float param_4,
                  float param_5)

{
  undefined **local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  undefined **local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004c92c0;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(float *)((int)this + 0x2ac) = param_2;
  *(float *)((int)this + 0x2b0) = param_3;
  *(float *)((int)this + 0x2b4) = param_4;
  *(float *)((int)this + 0x2b8) = param_5;
  local_34 = &PTR_Vec3_dtor_004cf300;
  local_30 = param_2 * (float)_DAT_004cf440;
  local_20 = &PTR_Vec3_dtor_004cf300;
  local_4._0_1_ = 1;
  local_4._1_3_ = 0;
  local_2c = param_3 * (float)_DAT_004cf440;
  local_28 = param_4 * (float)_DAT_004cf440;
  *(float *)((int)this + 0x1c4) = local_28;
  local_24 = param_5 * (float)_DAT_004cf438;
  *(float *)((int)this + 0x1bc) = local_30;
  *(float *)((int)this + 0x1c0) = local_2c;
  *(float *)((int)this + 0x1c8) = local_24;
  *(bool *)((int)this + 0x204) = local_24 != (float)_DAT_004cf3c8;
  local_1c = local_30;
  local_18 = local_2c;
  local_14 = local_28;
  local_10 = local_24;
  Matrix_Identity(&local_20);
  local_4 = (uint)local_4._1_3_ << 8;
  Matrix_Identity(&local_34);
  local_4 = 0xffffffff;
  Matrix_Identity(&param_1);
  ExceptionList = local_c;
  return;
}
