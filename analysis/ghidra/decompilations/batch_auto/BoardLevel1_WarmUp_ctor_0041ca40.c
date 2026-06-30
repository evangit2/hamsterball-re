/*
 * Function: LevelBoard_WarmUp_ctor
 * Address: 0x0041ca40
 * Signature: void * __thiscall LevelBoard_WarmUp_ctor(void *this,int param_1)
 *
 * Patterns: SEH frame, matrix math, board, level. Calls: LevelBoard_WarmUp_ctor, Board_ctor, Board, Vec3_Init, Matrix_Identity, LoadRaceData. Offsets: 12, Lines: 29
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall LevelBoard_WarmUp_ctor(void *this,int param_1)

{
  int iVar1;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca148;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  *(undefined ***)this = &PTR_LevelBoard_WarmUp_dtor_004d04a8;
  *(char **)((int)this + 0x868) = "Board (Warm-Up)";
  *(char **)((int)this + 0x29b4) = "WARM-UP RACE";
  local_4 = 0;
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  iVar1 = Vec3_Init(local_20,0x3f800000,0,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"BEGINNERRACE");
  *(char **)((int)this + 0x4344) = "Hamster Nation";
  ExceptionList = local_c;
  return this;
}
