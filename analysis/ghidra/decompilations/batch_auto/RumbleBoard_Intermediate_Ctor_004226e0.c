/*
 * Function: ArenaBoard_Intermediate_Ctor
 * Address: 0x004226e0
 * Signature: void * __thiscall ArenaBoard_Intermediate_Ctor(void *this,int param_1)
 *
 * Patterns: SEH frame, matrix math, board. Calls: ArenaBoard_Intermediate_Ctor, ArenaBoard_ctor, RumbleBoard, Vec3_Init, Matrix_Identity. Offsets: 10, Lines: 28
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall ArenaBoard_Intermediate_Ctor(void *this,int param_1)

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
  *(undefined ***)this = &PTR_ArenaBoard_DeletingDtor_004d15c0;
  *(char **)((int)this + 0x868) = "RumbleBoard (Intermediate Arena)";
  *(char **)((int)this + 0x4344) = "Gerbil Groove";
  *(char **)((int)this + 0x29b4) = "INTERMEDIATE ARENA";
  *(undefined1 *)((int)this + 0x4348) = 0;
  iVar1 = Vec3_Init(local_20,0,0,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  ExceptionList = local_c;
  return this;
}
