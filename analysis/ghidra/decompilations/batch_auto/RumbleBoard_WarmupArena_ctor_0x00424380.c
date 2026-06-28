
void * __thiscall ArenaBoard_WarmupArena_Ctor(void *this,int param_1)

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
  *(undefined ***)this = &PTR_ArenaBoard_DeletingDtor_004d1c80;
  *(char **)((int)this + 0x868) = "RumbleBoard (Warmup Arena)";
  iVar1 = Vec3_Init(local_20,0x3f000000,0x3f000000,0x3f000000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  *(char **)((int)this + 0x4344) = "Master Theme";
  *(char **)((int)this + 0x29b4) = "MASTER ARENA";
  *(undefined1 *)((int)this + 0x4348) = 0;
  ExceptionList = local_c;
  return this;
}

