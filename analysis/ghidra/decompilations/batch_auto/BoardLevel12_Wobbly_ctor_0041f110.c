/*
 * Function: BoardLevel12_Wobbly_ctor
 * Address: 0x0041f110
 * Signature: void * __thiscall BoardLevel12_Wobbly_ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, board, level. Calls: BoardLevel12_Wobbly_ctor, Board_ctor, Board, Vec3_Init, Matrix_Identity, LoadRaceData, operator_new, MeshWorld_ctor. Offsets: 24, Lines: 110
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall BoardLevel12_Wobbly_ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca4b5;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  iVar1 = *(int *)((int)this + 0x878);
  *(undefined ***)this = &PTR_BoardLevel12_Wobbly_dtor_004d0d38;
  *(char **)((int)this + 0x868) = "Board (Wobbly)";
  *(char **)((int)this + 0x29b4) = "WOBBLY RACE";
  local_4 = 0;
  if ((*(int *)(iVar1 + 0x23c) != 0) && (*(char *)(*(int *)(iVar1 + 0x220) + 0x10) == '\0')) {
    *(undefined1 *)(iVar1 + 0x857) = 1;
  }
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  iVar1 = Vec3_Init(local_20,0x3f1eb852,0x3f570a3d,0x3e99999a);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"WOBBLYRACE");
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 1;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level7-Wobbly1");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x436c) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level7-Wobbly2");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x4370) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level7-Wobbly3");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x4374) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level7-Wobbly4");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x4378) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 5;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level7-Wobbly5");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x437c) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 6;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level7-Wobbly6");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x4380) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,7);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level7-Wobbly7");
  }
  *(void **)((int)this + 0x4384) = pvVar2;
  *(char **)((int)this + 0x4344) = "Hamster Chase";
  ExceptionList = local_c;
  return this;
}
