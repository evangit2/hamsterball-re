
void * __thiscall Board_Glass_ctor(void *this,int param_1)

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
  local_4 = 0;
  *(undefined ***)this = &PTR_BoardLevel_Glass_scalar_dtor_004d1f90;
  *(char **)((int)this + 0x868) = "Board (Glass)";
  *(char **)((int)this + 0x29b4) = "GLASS RACE";
  iVar1 = Vec3_Init(local_20,0x3f800000,0,0x3f800000);
  *(undefined4 *)((int)this + 0x1508) = *(undefined4 *)(iVar1 + 4);
  *(undefined4 *)((int)this + 0x150c) = *(undefined4 *)(iVar1 + 8);
  *(undefined4 *)((int)this + 0x1510) = *(undefined4 *)(iVar1 + 0xc);
  *(undefined4 *)((int)this + 0x1514) = *(undefined4 *)(iVar1 + 0x10);
  Matrix_Identity(local_20);
  LoadRaceData(this,"GLASSRACE");
  *(char **)((int)this + 0x4344) = "Glass Theme";
  ExceptionList = local_c;
  return this;
}

