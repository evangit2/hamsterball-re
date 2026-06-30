/*
 * Function: ArenaBoard_Toob_ctor
 * Address: 0x004234e0
 * Signature: void * __thiscall ArenaBoard_Toob_ctor(void *this,int param_1)
 *
 * Patterns: SEH frame, matrix math, board. Calls: ArenaBoard_Toob_ctor, ArenaBoard_ctor, _eh_vector_constructor_iterator_, CONCAT31, RumbleBoard, Vec3_Init, Matrix_Identity. Offsets: 11, Lines: 30
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall ArenaBoard_Toob_ctor(void *this,int param_1)

{
  int iVar1;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004caa13;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  ArenaBoard_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_ArenaBoard_CollSlices_scalar_dtor_004d1a40;
  _eh_vector_constructor_iterator_((void *)((int)this + 0x47e0),0x418,5,FUN_0040a870,Vec3List_Free);
  local_4 = CONCAT31(local_4._1_3_,1);
  *(char **)((int)this + 0x868) = "RumbleBoard (Toob Arena)";
  *(char **)((int)this + 0x4344) = "Rodenthood";
  iVar1 = Vec3_Init(local_20,0x3f000000,0x3f000000,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  *(char **)((int)this + 0x29b4) = "TOOB ARENA";
  *(undefined1 *)((int)this + 0x4348) = 0;
  ExceptionList = local_c;
  return this;
}
