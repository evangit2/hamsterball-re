/*
 * Function: BoardLevel_Toob_Ctor
 * Address: 0x0041f4b0
 * Signature: void * __thiscall BoardLevel_Toob_Ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, board, level. Calls: BoardLevel_Toob_Ctor, Board_ctor, _eh_vector_constructor_iterator_, Board, Vec3_Init, Matrix_Identity, LoadRaceData, operator_new. Offsets: 26, Lines: 93
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall BoardLevel_Toob_Ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca53a;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_BoardLevel_Toob_dtor_004d0e78;
  _eh_vector_constructor_iterator_((void *)((int)this + 0x438c),0x418,8,FUN_0040a870,Vec3List_Free);
  iVar1 = *(int *)((int)this + 0x878);
  *(char **)((int)this + 0x868) = "Board (Toob)";
  *(char **)((int)this + 0x29b4) = "TOOB RACE";
  local_4._0_1_ = 1;
  if ((*(int *)(iVar1 + 0x23c) != 0) && (*(char *)(*(int *)(iVar1 + 0x220) + 0x10) == '\0')) {
    *(undefined1 *)(iVar1 + 0x856) = 1;
  }
  *(undefined4 *)((int)this + 0x870) = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1dc);
  iVar1 = Vec3_Init(local_20,0x3f000000,0x3f000000,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"TOOBRACE");
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level8-Spinny");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x436c) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level8-Saw");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x4370) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level8-Fallout");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x4374) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 5;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level8-Blockdawg1");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x4378) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,6);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\Level8-Blockdawg2");
  }
  *(undefined4 *)((int)this + 0x4380) = 0;
  *(undefined4 *)((int)this + 0x4384) = 0;
  *(undefined4 *)((int)this + 0x4388) = 0;
  *(void **)((int)this + 0x437c) = pvVar2;
  *(char **)((int)this + 0x4344) = "Rodenthood";
  ExceptionList = local_c;
  return this;
}
