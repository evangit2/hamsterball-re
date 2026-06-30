/*
 * Function: LevelBoard_Intermediate_ctor
 * Address: 0x0041cb20
 * Signature: void * __thiscall LevelBoard_Intermediate_ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, collision, board, level. Calls: LevelBoard_Intermediate_ctor, Board_ctor, Board, operator_new, MeshWorld_ctor, CollisionLevel_ctorWithLevel, TipperVisual_Attach, Vec3_Init. Offsets: 16, Lines: 54
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall LevelBoard_Intermediate_ctor(void *this,int param_1)

{
  void *pvVar1;
  int iVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca17e;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_LevelBoard_Intermediate_dtor_004d05a0;
  *(char **)((int)this + 0x868) = "Board (Intermediate)";
  *(char **)((int)this + 0x29b4) = "INTERMEDIATE RACE";
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level2-Bridge");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x436c) = pvVar1;
  pvVar1 = operator_new(0x10d0);
  local_4._0_1_ = 2;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = CollisionLevel_ctorWithLevel(pvVar1,*(int *)((int)this + 0x436c));
  }
  local_4 = (uint)local_4._1_3_ << 8;
  *(void **)((int)this + 0x4370) = pvVar1;
  TipperVisual_Attach(pvVar1,*(int *)((int)this + 0x436c));
  *(undefined4 *)((int)this + 0x4380) = 0x42340000;
  *(undefined4 *)((int)this + 0x4384) = 0;
  *(undefined4 *)((int)this + 0x4388) = 0x32;
  iVar2 = Vec3_Init(local_20,0,0,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar2 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar2 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar2 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar2 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"INTERMEDIATERACE");
  *(char **)((int)this + 0x4344) = "Gerbil Groove";
  ExceptionList = local_c;
  return this;
}
