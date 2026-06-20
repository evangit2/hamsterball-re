/*
 * Function: BoardLevel9_Odd_ctor
 * Address: 0x0041ed80
 * Signature: void * __thiscall BoardLevel9_Odd_ctor(void *this,int param_1)
 *
 * Patterns: SEH frame, matrix math, board, level. Calls: BoardLevel9_Odd_ctor, Board_ctor, Board, Vec3_Init, Matrix_Identity, LoadRaceData. Offsets: 16, Lines: 33
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall BoardLevel9_Odd_ctor(void *this,int param_1)

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
  iVar1 = *(int *)((int)this + 0x878);
  *(undefined ***)this = &PTR_BoardLevel9_Odd_dtor_004d0bc0;
  *(char **)((int)this + 0x868) = "Board (Odd)";
  *(char **)((int)this + 0x29b4) = "ODD RACE";
  local_4 = 0;
  if ((*(int *)(iVar1 + 0x23c) != 0) && (*(char *)(*(int *)(iVar1 + 0x220) + 0x10) == '\0')) {
    *(undefined1 *)(iVar1 + 0x855) = 1;
  }
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  iVar1 = Vec3_Init(local_20,0x3f800000,0x3f000000,0);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"ODDRACE");
  *(char **)((int)this + 0x4344) = "Ninja Hamster";
  ExceptionList = local_c;
  return this;
}
