/*
 * Function: ArenaBoard_Dizzy_ctor
 * Address: 0x00422790
 * Signature: void * __thiscall ArenaBoard_Dizzy_ctor(void *this,int param_1)
 *
 * Patterns: SEH frame, matrix math, board. Calls: ArenaBoard_Dizzy_ctor, ArenaBoard_ctor, RumbleBoard, Vec3_Init, Matrix_Identity. Offsets: 11, Lines: 29
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall ArenaBoard_Dizzy_ctor(void *this,int param_1)

{
  int iVar1;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca8e8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  ArenaBoard_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_ArenaBoard_Expert_DeletingDtor_004d1680;
  *(char **)((int)this + 0x868) = "RumbleBoard (Dizzy Arena)";
  *(char **)((int)this + 0x4344) = "Dizzy!";
  *(char **)((int)this + 0x29b4) = "DIZZY ARENA";
  *(undefined1 *)((int)this + 0x4348) = 0;
  iVar1 = Vec3_Init(local_20,0,0x3f800000,0);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  *(undefined4 *)((int)this + 0x47e0) = 0;
  ExceptionList = local_c;
  return this;
}
