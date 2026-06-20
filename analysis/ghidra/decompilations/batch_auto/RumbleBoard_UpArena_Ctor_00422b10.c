/*
 * Function: RumbleBoard_UpArena_Ctor
 * Address: 0x00422b10
 * Signature: void * __thiscall RumbleBoard_UpArena_Ctor(void *this,int param_1)
 *
 * Patterns: allocates, SEH frame, matrix math, board. Calls: RumbleBoard_UpArena_Ctor, RumbleBoard_ctor, RumbleBoard, Vec3_Init, Matrix_Identity, operator_new, CONCAT31, MeshWorld_ctor. Offsets: 13, Lines: 39
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall RumbleBoard_UpArena_Ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cab53;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  RumbleBoard_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_RumbleBoard_ToobArena_DeletingDtor_004d17f8;
  *(char **)((int)this + 0x868) = "RumbleBoard (Up Arena)";
  *(char **)((int)this + 0x4344) = "Up Race";
  *(char **)((int)this + 0x29b4) = "UP ARENA";
  *(undefined1 *)((int)this + 0x4348) = 0;
  iVar1 = Vec3_Init(local_20,0x3f800000,0,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  pvVar2 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,1);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\levelup-lifter");
  }
  *(void **)((int)this + 0x47e0) = pvVar2;
  ExceptionList = local_c;
  return this;
}
