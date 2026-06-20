
void * __thiscall Board_NeonRace_ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004caada;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_BoardLevel14_RaceOfAges_Scene_scalar_dtor_004d1df0;
  *(char **)((int)this + 0x868) = "Board (Dark)";
  *(char **)((int)this + 0x29b4) = "NEON RACE";
  iVar1 = Vec3_Init(local_20,0x3f800000,0x3f800000,0);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"NEONRACE");
  *(char **)((int)this + 0x4344) = "Neon Theme";
  *(undefined4 *)((int)this + 0x436c) = 0;
  *(undefined4 *)((int)this + 0x4370) = 0;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 1;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\LevelDark-NeonPlatform");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x4374) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\LevelDark-DFloor1");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x4378) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 3;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\LevelDark-DFloor2");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x437c) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 4;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\LevelDark-DFloor3");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x4380) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4._0_1_ = 5;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\LevelDark-DFloor4");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x4384) = pvVar2;
  pvVar2 = operator_new(0x10d0);
  local_4 = CONCAT31(local_4._1_3_,6);
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = MeshWorld_ctor(pvVar2,*(undefined4 *)(*(int *)((int)this + 0x878) + 0x174),
                            "Levels\\LevelDark-Trode");
  }
  *(void **)((int)this + 0x4388) = pvVar2;
  *(undefined4 *)((int)this + 0x4390) = 0;
  ExceptionList = local_c;
  return this;
}

