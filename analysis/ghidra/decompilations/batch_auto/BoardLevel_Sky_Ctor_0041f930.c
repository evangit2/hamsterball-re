/*
 * Function: LevelBoard_Sky_ctor
 * Address: 0x0041f930
 * Signature: void * __thiscall LevelBoard_Sky_ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, board, level. Calls: LevelBoard_Sky_ctor, Board_ctor, AthenaList_Init, Board, Vec3_Init, Matrix_Identity, LoadRaceData, operator_new. Offsets: 27, Lines: 104
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall LevelBoard_Sky_ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca5d8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_BoardLevel_Generic_dtor_004d0fc8;
  AthenaList_Init((void *)((int)this + 0x4394),0);
  iVar1 = *(int *)((int)this + 0x878);
  *(char **)((int)this + 0x868) = "Board (Sky)";
  *(char **)((int)this + 0x29b4) = "SKY RACE";
  local_4._0_1_ = 1;
  if ((*(int *)(iVar1 + 0x23c) != 0) && (*(char *)(*(int *)(iVar1 + 0x220) + 0x10) == '\0')) {
    *(undefined1 *)(iVar1 + 0x858) = 1;
  }
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  iVar1 = Vec3_Init(local_20,0,0x3f000000,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"SKYRACE");
  pvVar2 = operator_new(0x18);
  local_4._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshNode_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                           "meshes\\skypillar");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x436c) = pvVar2;
  pvVar2 = operator_new(0x18);
  local_4._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshNode_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                           "meshes\\magnifyingglass");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x4370) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\level9-popcylinder1");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x4384) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 5;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\level9-popcylinder2");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x4388) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 6;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\level9-trapdoor");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x438c) = pvVar2;
  pvVar2 = operator_new(0xd4);
  local_4 = CONCAT31(local_4._1_3_,7);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = Sprite_ctor(pvVar2,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "textures\\clouds.png");
  }
  *(undefined4 *)((int)this + 0x47ac) = 0;
  *(undefined4 *)((int)this + 0x47f0) = 0;
  *(undefined4 *)((int)this + 0x47f4) = 0;
  *(void **)((int)this + 0x4374) = pvVar2;
  *(char **)((int)this + 0x4344) = "Bucky Break";
  ExceptionList = local_c;
  return this;
}
