/*
 * Function: LevelBoard_Up_ctor
 * Address: 0x00420390
 * Signature: void * __thiscall LevelBoard_Up_ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, board, level. Calls: LevelBoard_Up_ctor, Board_ctor, AthenaList_Init, Board, Vec3_Init, Matrix_Identity, LoadRaceData, operator_new. Offsets: 21, Lines: 68
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall LevelBoard_Up_ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca677;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_BoardLevel_Generic_dtor3_004d11a0;
  AthenaList_Init((void *)((int)this + 0x436c),0);
  *(char **)((int)this + 0x868) = "Board (Up)";
  *(char **)((int)this + 0x29b4) = "UP RACE";
  local_4._0_1_ = 1;
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  iVar1 = Vec3_Init(local_20,0x3f800000,0,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"UPRACE");
  iVar1 = *(int *)((int)this + 0x878);
  *(char **)((int)this + 0x4344) = "Up Race";
  if ((*(int *)(iVar1 + 0x23c) != 0) && (*(char *)(*(int *)(iVar1 + 0x220) + 0x10) == '\0')) {
    *(undefined1 *)(iVar1 + 0x853) = 1;
  }
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\levelup-lifter");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x4784) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\levelup-speedcylinder");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x4788) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,4);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\levelup-button");
  }
  *(void **)((int)this + 0x478c) = pvVar2;
  ExceptionList = local_c;
  return this;
}
