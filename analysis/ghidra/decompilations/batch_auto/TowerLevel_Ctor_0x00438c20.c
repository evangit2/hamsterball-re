
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * __thiscall
TowerLevel_Ctor(void *this,int param_1,float param_2,undefined4 param_3,undefined4 param_4,
               float param_5)

{
  float *pfVar1;
  float *this_00;
  float fVar2;
  int local_50 [14];
  void *pvStack_18;
  undefined1 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  float local_4;
  
  local_4 = -NAN;
  puStack_8 = &LAB_004cb9e0;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  Level_ctor(this,*(undefined4 *)(*(int *)(param_1 + 0x878) + 0x174));
  pfVar1 = (float *)((int)this + 0x10d4);
  local_4 = 0.0;
  *(undefined ***)this = &PTR_Tower_Level_scalar_dtor_004d5180;
  if (pfVar1 != &param_2) {
    *pfVar1 = param_2;
    *(undefined4 *)((int)this + 0x10d8) = param_3;
    *(undefined4 *)((int)this + 0x10dc) = param_4;
  }
  fVar2 = _DAT_004cf44c - param_5;
  *(int *)((int)this + 0x10d0) = param_1;
  *(float *)((int)this + 0x10e0) = fVar2;
  *(undefined1 *)(*(int *)((int)this + 0x480) + 0x10d0) = 1;
  *(undefined4 *)((int)this + 0x10e8) = 0x3f000000;
  *(undefined1 *)((int)this + 0x10ee) = 1;
  *(undefined4 *)((int)this + 0x10f0) = 0x43fa0000;
  *(undefined4 *)((int)this + 0x10fc) = 0;
  *(undefined4 *)((int)this + 0x10f8) = 0;
  Timer_Init(local_50);
  local_4 = (float)CONCAT31(local_4._1_3_,1);
  Gfx_ScaleX(*(float *)((int)this + 0x10e0));
  if (&param_2 != pfVar1) {
    param_2 = *pfVar1;
    param_3 = *(undefined4 *)((int)this + 0x10d8);
    param_4 = *(undefined4 *)((int)this + 0x10dc);
  }
  (**(code **)(local_50[0] + 8))(param_2,param_3,param_4);
  local_4 = 420.0;
  if (*(char *)(*(int *)(*(int *)((int)this + 0x10d0) + 0x878) + 0x237) != '\0') {
    local_4 = 100.0;
  }
  Matrix_TransformVec3((float *)&stack0xffffffa8,&local_4);
  *(float *)((int)this + 0x1104) = local_4;
  *(undefined4 *)((int)this + 0x1108) = 0;
  local_4 = -550.0;
  Matrix_TransformVec3((float *)&stack0xffffffa8,&local_4);
  *(float *)((int)this + 0x1124) = local_4;
  *(undefined4 *)((int)this + 0x1128) = 0x443b8000;
  local_4 = -550.0;
  Matrix_TransformVec3((float *)&stack0xffffffa8,&local_4);
  *(float *)((int)this + 0x1144) = local_4;
  *(undefined4 *)((int)this + 0x1148) = 0xc43b8000;
  local_4 = -10.0;
  Matrix_TransformVec3((float *)&stack0xffffffa8,&local_4);
  this_00 = (float *)((int)this + 0x1164);
  if (this_00 != &local_4) {
    *this_00 = local_4 - *pfVar1;
    *(float *)((int)this + 0x1168) = 0.0 - *(float *)((int)this + 0x10d8);
    *(float *)((int)this + 0x116c) = 0.0 - *(float *)((int)this + 0x10dc);
  }
  Vec3_NormalizeAndScale(this_00,1.0);
  pfVar1 = (float *)((int)this + 0x1170);
  if (pfVar1 != &local_4) {
    *pfVar1 = -200.0;
    *(undefined4 *)((int)this + 0x1174) = 0;
    *(undefined4 *)((int)this + 0x1178) = 0;
  }
  Matrix_TransformVec3((float *)&stack0xffffffa8,pfVar1);
  pfVar1 = (float *)((int)this + 0x117c);
  if (pfVar1 != &local_4) {
    *pfVar1 = 0.0;
    *(undefined4 *)((int)this + 0x1180) = 0;
    *(undefined4 *)((int)this + 0x1184) = 0;
  }
  Matrix_TransformVec3((float *)&stack0xffffffa8,pfVar1);
  puStack_8 = (undefined1 *)RNG_Rand(&PTR_OBJ_VTABLE,0x168,'\0');
  *(undefined1 *)((int)this + 0x10ec) = 0;
  *(undefined1 *)((int)this + 0x10ed) = 0;
  *(float *)((int)this + 0x10e4) = (float)(int)puStack_8;
  *(undefined1 *)((int)this + 0x10f4) = 0;
  *(undefined1 *)((int)this + 0x10f5) = 0;
  uStack_10 = 0;
  Timer_Cleanup((undefined4 *)&stack0xffffffa4);
  ExceptionList = pvStack_18;
  return this;
}

