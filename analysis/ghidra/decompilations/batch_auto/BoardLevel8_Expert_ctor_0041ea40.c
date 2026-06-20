/*
 * Function: BoardLevel8_Expert_ctor
 * Address: 0x0041ea40
 * Signature: void * __thiscall BoardLevel8_Expert_ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, collision, board, level. Calls: BoardLevel8_Expert_ctor, Board_ctor, AthenaList_Init, Board, Vec3_Init, Matrix_Identity, LoadRaceData, operator_new. Offsets: 25, Lines: 93
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall BoardLevel8_Expert_ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca3e9;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_BoardLevel8_Expert_dtor_004d0b00;
  AthenaList_Init((void *)((int)this + 0x4380),0);
  local_4._0_1_ = 1;
  AthenaList_Init((void *)((int)this + 0x4798),0);
  local_4._0_1_ = 2;
  AthenaList_Init((void *)((int)this + 0x4bbc),0);
  iVar1 = *(int *)((int)this + 0x878);
  *(char **)((int)this + 0x868) = "Board (Expert)";
  *(char **)((int)this + 0x29b4) = "EXPERT RACE";
  local_4._0_1_ = 3;
  if ((*(int *)(iVar1 + 0x23c) != 0) && (*(char *)(*(int *)(iVar1 + 0x220) + 0x10) == '\0')) {
    *(undefined1 *)(iVar1 + 0x854) = 1;
  }
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  iVar1 = Vec3_Init(local_20,0x3f800000,0,0);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"EXPERTRACE");
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level5-Bridge");
  }
  local_4._0_1_ = 3;
  *(void **)((int)this + 0x4378) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 5;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = CollisionLevel_ctorWithLevel(pvVar2,*(int *)((int)this + 0x4378));
  }
  local_4._0_1_ = 3;
  *(void **)((int)this + 0x437c) = pvVar2;
  pvVar2 = operator_new(0x18);
  local_4._0_1_ = 6;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshNode_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                           "meshes\\hammyjudge");
  }
  local_4._0_1_ = 3;
  *(void **)((int)this + 0x4bb0) = pvVar2;
  pvVar2 = operator_new(0x18);
  local_4._0_1_ = 7;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshNode_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                           "meshes\\hammyjudge");
  }
  local_4._0_1_ = 3;
  *(void **)((int)this + 0x4bb4) = pvVar2;
  pvVar2 = operator_new(0x18);
  local_4 = CONCAT31(local_4._1_3_,8);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshNode_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                           "meshes\\hammyjudge");
  }
  *(void **)((int)this + 0x4bb8) = pvVar2;
  *(char **)((int)this + 0x4344) = "Fight!";
  ExceptionList = local_c;
  return this;
}
