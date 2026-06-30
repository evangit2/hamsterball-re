/*
 * Function: LevelBoard_Tower_ctor
 * Address: 0x0041e340
 * Signature: void * __thiscall LevelBoard_Tower_ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, rendering, board, level. Calls: LevelBoard_Tower_ctor, Board_ctor, AthenaList_Init, Board, Vec3_Init, Matrix_Identity, LoadRaceData, operator_new. Offsets: 31, Lines: 121
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall LevelBoard_Tower_ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca31d;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_LevelBoard_Tower_dtor_004d0a08;
  AthenaList_Init((void *)((int)this + 0x43b8),0);
  local_4._0_1_ = 1;
  AthenaList_Init((void *)((int)this + 0x47d0),0);
  local_4._0_1_ = 2;
  AthenaList_Init((void *)((int)this + 0x4be8),0);
  local_4._0_1_ = 3;
  AthenaList_Init((void *)((int)this + 0x5000),0);
  iVar1 = *(int *)((int)this + 0x878);
  *(char **)((int)this + 0x868) = "Board (Tower)";
  *(char **)((int)this + 0x29b4) = "TOWER RACE";
  local_4._0_1_ = 4;
  if ((*(int *)(iVar1 + 0x23c) != 0) && (*(char *)(*(int *)(iVar1 + 0x220) + 0x10) == '\0')) {
    *(undefined1 *)(iVar1 + 0x852) = 1;
  }
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  iVar1 = Vec3_Init(local_20,0x3f800000,0x3f400000,0);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"TOWERRACE");
  *(char **)((int)this + 0x4344) = "Happy Rush";
  *(undefined4 *)((int)this + 0x43a4) = 0;
  *(undefined4 *)((int)this + 0x43a0) = 0;
  *(undefined4 *)((int)this + 0x43a8) = 0;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 5;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level4-Catapult");
  }
  local_4._0_1_ = 4;
  *(void **)((int)this + 0x436c) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 6;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level4-Drawbridge");
  }
  local_4._0_1_ = 4;
  *(void **)((int)this + 0x4370) = pvVar2;
  pvVar2 = operator_new(0x18);
  local_4._0_1_ = 7;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshNode_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                           "Meshes\\YellowLink");
  }
  local_4._0_1_ = 4;
  *(void **)((int)this + 0x4374) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 8;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level4-Mace");
  }
  local_4._0_1_ = 4;
  *(void **)((int)this + 0x4378) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 9;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level4-Windmill");
  }
  local_4._0_1_ = 4;
  *(void **)((int)this + 0x437c) = pvVar2;
  pvVar2 = operator_new(0x18);
  local_4._0_1_ = 10;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshNode_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                           "Meshes\\Chomper");
  }
  local_4._0_1_ = 4;
  *(void **)((int)this + 0x4390) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,0xb);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level4-Turret");
  }
  *(void **)((int)this + 0x43b4) = pvVar2;
  ExceptionList = local_c;
  return this;
}
