/*
 * Function: LevelBoard_Beginner_ctor
 * Address: 0x004200e0
 * Signature: void * __thiscall LevelBoard_Beginner_ctor(void *this,int param_1)
 *
 * Patterns: SEH frame, matrix math, board, level. Calls: LevelBoard_Beginner_ctor, Board_ctor, _eh_vector_constructor_iterator_, Board, CONCAT31, Vec3_Init, Matrix_Identity, LoadRaceData. Offsets: 13, Lines: 31
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall LevelBoard_Beginner_ctor(void *this,int param_1)

{
  int iVar1;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca633;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_BoardLevel_Generic_dtor2_004d1098;
  _eh_vector_constructor_iterator_((void *)((int)this + 0x436c),0x418,8,FUN_0040a870,Vec3List_Free);
  *(char **)((int)this + 0x868) = "Board (Beginner)";
  *(char **)((int)this + 0x29b4) = "BEGINNER RACE";
  local_4 = CONCAT31(local_4._1_3_,1);
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  iVar1 = Vec3_Init(local_20,0x3f800000,0x3f400000,0x3e800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"CASCADERACE");
  *(char **)((int)this + 0x4344) = "Cascade Race";
  ExceptionList = local_c;
  return this;
}
