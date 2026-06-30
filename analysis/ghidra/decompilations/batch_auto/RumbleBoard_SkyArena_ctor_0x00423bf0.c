
void * __thiscall ArenaBoard_Sky_ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004caa57;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  ArenaBoard_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_ArenaBoard9_PopCylinder_ScalarDtor_004d1bd8;
  AthenaList_Init((void *)((int)this + 0x47ec),0);
  local_4._0_1_ = 1;
  *(char **)((int)this + 0x868) = "RumbleBoard (Sky Arena)";
  *(char **)((int)this + 0x4344) = "Bucky Break";
  *(char **)((int)this + 0x29b4) = "SKY ARENA";
  *(undefined1 *)((int)this + 0x4348) = 0;
  iVar1 = Vec3_Init(local_20,0,0x3f000000,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  pvVar2 = operator_new(0x18);
  local_4._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshNode_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                           "meshes\\skypillar");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x47e0) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\level9-popcylinder1");
  }
  local_4._0_1_ = 1;
  *(void **)((int)this + 0x47e4) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,4);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "levels\\level9-popcylinder2");
  }
  *(void **)((int)this + 0x47e8) = pvVar2;
  *(undefined4 *)((int)this + 0x4cf4) = 0;
  *(undefined4 *)((int)this + 0x4cf8) = 0;
  ExceptionList = local_c;
  return this;
}

