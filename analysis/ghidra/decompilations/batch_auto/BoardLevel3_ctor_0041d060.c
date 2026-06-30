/*
 * Function: LevelBoard_Dizzy_ctor
 * Address: 0x0041d060
 * Signature: LevelBoard_Dizzy_ctor(...)
 *
 * Patterns: allocates, SEH frame, matrix math, audio, collision, rendering, scene, board, level. Calls: meshes, LevelBoard_Dizzy_ctor, Board_ctor, AthenaList_Init, BoardLevel3, Board, operator_new, MeshWorld_ctor. Offsets: 31, Lines: 139
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* LevelBoard_Dizzy_ctor: "Dizzy Race" level. Loads sub-meshes (Tipper/WaterWheel/Swirl/Gluebie) as
   MeshWorld+CollisionLevel pairs. Race data="DIZZYRACE". See
   decompilations/scene/decomp_object_factory.c */

void * __thiscall LevelBoard_Dizzy_ctor(void *this,int param_1)

{
  void *pvVar1;
  int iVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca201;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_LevelBoard_Dizzy_dtor_004d0890;
  AthenaList_Init((void *)((int)this + 0x4378),0);
  local_4._0_1_ = 1;
  AthenaList_Init((void *)((int)this + 0x4790),0);
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(1)";
  *(char **)((int)this + 0x868) = "Board (Dizzy)";
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(2)";
  *(char **)((int)this + 0x29b4) = "DIZZY RACE";
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(3)";
  iVar2 = *(int *)((int)this + 0x878);
  local_4._0_1_ = 2;
  if ((*(int *)(iVar2 + 0x23c) != 0) && (*(char *)(*(int *)(iVar2 + 0x220) + 0x10) == '\0')) {
    *(undefined1 *)(iVar2 + 0x851) = 1;
  }
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(4)";
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 3;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level3-Tipper");
  }
  *(void **)((int)this + 0x436c) = pvVar1;
  local_4._0_1_ = 2;
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(5)";
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 4;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(pvVar1,*(int *)((int)this + 0x436c));
  }
  *(void **)((int)this + 0x4370) = pvVar1;
  local_4._0_1_ = 2;
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(6)";
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 5;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level3-WaterWheel");
  }
  local_4._0_1_ = 2;
  *(void **)((int)this + 0x4ba8) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 6;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(pvVar1,*(int *)((int)this + 0x4ba8));
  }
  *(void **)((int)this + 0x4bac) = pvVar1;
  local_4._0_1_ = 2;
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(7)";
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 7;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level3-Swirl");
  }
  *(void **)((int)this + 0x4bc4) = pvVar1;
  local_4._0_1_ = 2;
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(8)";
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 8;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(pvVar1,*(int *)((int)this + 0x4bc4));
  }
  *(void **)((int)this + 0x4bc8) = pvVar1;
  local_4._0_1_ = 2;
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(9)";
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 9;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level3-Gluebie");
  }
  *(void **)((int)this + 0x4374) = pvVar1;
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(10)";
  local_4 = CONCAT31(local_4._1_3_,2);
  iVar2 = Sound_GetNextChannel(*(int *)(*(int *)((int)this + 0x878) + 0x490));
  *(int *)((int)this + 0x4bdc) = iVar2;
  if (iVar2 != 0) {
    Scene_RenderIfVisible(iVar2);
  }
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(11)";
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(12)";
  *(undefined4 *)((int)this + 0x4bc0) = 0;
  *(undefined4 *)((int)this + 0x4bd8) = 0;
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(13)";
  iVar2 = Vec3_Init(local_20,0,0x3f800000,0);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar2 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar2 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar2 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar2 + 0x10);
  Matrix_Identity(local_20);
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(14)";
  LoadRaceData(this,"DIZZYRACE");
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(15)";
  *(char **)((int)this + 0x4344) = "Dizzy!";
  *(char **)(*(int *)((int)this + 0x878) + 0x208) = "BoardLevel3::BoardLevel3(16)";
  ExceptionList = local_c;
  return this;
}
