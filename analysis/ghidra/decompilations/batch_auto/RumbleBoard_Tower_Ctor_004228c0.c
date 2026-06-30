/*
 * Function: ArenaBoard_Tower_ctor
 * Address: 0x004228c0
 * Signature: void * __thiscall ArenaBoard_Tower_ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, board, level. Calls: ArenaBoard_Tower_ctor, ArenaBoard_ctor, AthenaList_Init, RumbleBoard, Vec3_Init, Matrix_Identity, operator_new, MeshWorld_ctor. Offsets: 17, Lines: 65
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall ArenaBoard_Tower_ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca975;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  ArenaBoard_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_ArenaBoard_Odd_DeletingDtor_004d1740;
  AthenaList_Init((void *)((int)this + 0x47e4),0);
  local_4._0_1_ = 1;
  AthenaList_Init((void *)((int)this + 0x4c00),0);
  local_4._0_1_ = 2;
  *(char **)((int)this + 0x868) = "RumbleBoard (Tower Arena)";
  *(char **)((int)this + 0x4344) = "Happy Rush";
  *(char **)((int)this + 0x29b4) = "TOWER ARENA";
  *(undefined1 *)((int)this + 0x4348) = 0;
  iVar1 = Vec3_Init(local_20,0x3f800000,0x3f400000,0);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level4-Mace");
  }
  local_4._0_1_ = 2;
  *(void **)((int)this + 0x47e0) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level4-Catapult");
  }
  local_4._0_1_ = 2;
  *(void **)((int)this + 0x4bfc) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,5);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level4-Turret");
  }
  *(void **)((int)this + 0x5018) = pvVar2;
  ExceptionList = local_c;
  return this;
}
