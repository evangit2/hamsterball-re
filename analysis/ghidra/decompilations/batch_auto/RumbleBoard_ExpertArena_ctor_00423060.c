/*
 * Function: ArenaBoard_Expert_ctor
 * Address: 0x00423060
 * Signature: void * __thiscall ArenaBoard_Expert_ctor(void *this,int param_1)
 *
 * Patterns: SEH frame, matrix math, board. Calls: ArenaBoard_Expert_ctor, ArenaBoard_ctor, AthenaList_Init, CONCAT31, RumbleBoard, Vec3_Init, Matrix_Identity. Offsets: 12, Lines: 31
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall ArenaBoard_Expert_ctor(void *this,int param_1)

{
  int iVar1;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca9e6;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  ArenaBoard_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_ArenaBoard_CollSlicesArena_DeletingDtor_004d18c8;
  AthenaList_Init((void *)((int)this + 0x47e0),0);
  local_4 = CONCAT31(local_4._1_3_,1);
  *(char **)((int)this + 0x868) = "RumbleBoard (Expert Arena)";
  *(char **)((int)this + 0x4344) = "Fight!";
  *(char **)((int)this + 0x29b4) = "EXPERT ARENA";
  *(undefined1 *)((int)this + 0x4348) = 0;
  iVar1 = Vec3_Init(local_20,0x3f800000,0,0);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  *(undefined4 *)((int)this + 0x4bf8) = 1000;
  ExceptionList = local_c;
  return this;
}
