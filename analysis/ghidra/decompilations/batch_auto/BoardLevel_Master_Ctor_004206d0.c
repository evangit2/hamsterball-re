/*
 * Function: BoardLevel_Master_Ctor
 * Address: 0x004206d0
 * Signature: void * __thiscall BoardLevel_Master_Ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, collision, board, level. Calls: BoardLevel_Master_Ctor, Board_ctor, _eh_vector_constructor_iterator_, AthenaList_Init, Board, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel. Offsets: 40, Lines: 206
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall BoardLevel_Master_Ctor(void *this,int param_1)

{
  void *pvVar1;
  int *piVar2;
  int iVar3;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca7a5;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_BoardLevel_Generic_dtor4_004d12b0;
  _eh_vector_constructor_iterator_((void *)((int)this + 0x439c),0x418,4,FUN_0040a870,Vec3List_Free);
  local_4._0_1_ = 1;
  AthenaList_Init((void *)((int)this + 0x5428),0);
  local_4._0_1_ = 2;
  AthenaList_Init((void *)((int)this + 0x584c),0);
  local_4._0_1_ = 3;
  AthenaList_Init((void *)((int)this + 0x5c64),0);
  local_4._0_1_ = 4;
  AthenaList_Init((void *)((int)this + 0x6080),0);
  iVar3 = *(int *)((int)this + 0x878);
  *(char **)((int)this + 0x868) = "Board (Master)";
  *(char **)((int)this + 0x29b4) = "MASTER RACE";
  local_4._0_1_ = 5;
  if ((*(int *)(iVar3 + 0x23c) != 0) && (*(char *)(*(int *)(iVar3 + 0x220) + 0x10) == '\0')) {
    *(undefined1 *)(iVar3 + 0x859) = 1;
  }
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 6;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level2-Bridge");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x436c) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 7;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(pvVar1,*(int *)((int)this + 0x436c));
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x4370) = pvVar1;
  TipperVisual_Attach(pvVar1,*(int *)((int)this + 0x436c));
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 8;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level10-2PBridge");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x4374) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 9;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(pvVar1,*(int *)((int)this + 0x4374));
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x4378) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 10;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level3-Tipper");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x4394) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 0xb;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(pvVar1,*(int *)((int)this + 0x4394));
  }
  *(void **)((int)this + 0x4398) = pvVar1;
  local_4._0_1_ = 5;
  Level_AssignTexturesAndScales(this,*(int **)((int)this + 0x436c));
  Level_AssignTexturesAndScales(this,*(int **)((int)this + 0x4394));
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 0xc;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level10-Bridge1");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x5410) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 0xd;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level10-Bridge2");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x5414) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 0xe;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\level9-popcylinder1");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x5420) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 0xf;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\level9-popcylinder2");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x5424) = pvVar1;
  Level_AssignTexturesAndScales(this,*(int **)((int)this + 0x5420));
  Level_AssignTexturesAndScales(this,*(int **)((int)this + 0x5424));
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 0x10;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level8-Blockdawg1");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x5840) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 0x11;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level8-Blockdawg2");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x5844) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 0x12;
  if (pvVar1 == (void *)0x0) {
    piVar2 = (int *)0x0;
  }
  else {
    piVar2 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level4-Catapult");
  }
  local_4._0_1_ = 5;
  *(int **)((int)this + 0x5848) = piVar2;
  Level_AssignTexturesAndScales(this,piVar2);
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 0x13;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level3-Gluebie");
  }
  *(void **)((int)this + 0x607c) = pvVar1;
  *(undefined4 *)((int)this + 0x4388) = 0x42340000;
  *(undefined4 *)((int)this + 0x438c) = 0;
  *(undefined4 *)((int)this + 0x4390) = 0x32;
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  local_4 = CONCAT31(local_4._1_3_,5);
  iVar3 = Vec3_Init(local_20,0x3f000000,0x3f000000,0x3f000000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar3 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar3 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar3 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar3 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"MASTERRACE");
  *(char **)((int)this + 0x4344) = "Master Theme";
  *(undefined4 *)((int)this + 0x29c0) = 0x449c4000;
  ExceptionList = local_c;
  return this;
}
