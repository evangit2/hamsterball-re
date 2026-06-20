
void * __thiscall
CollisionLevel_Spatial_Ctor
          (void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  *(undefined ***)this = &PTR_Pendulum_DeletingDtor_004d569c;
  AthenaList_Init((void *)((int)this + 0x2c),0);
  *(undefined4 *)((int)this + 4) = param_1;
  if ((undefined4 *)((int)this + 8) != &param_2) {
    *(undefined4 *)((int)this + 8) = param_2;
    *(undefined4 *)((int)this + 0xc) = param_3;
    *(undefined4 *)((int)this + 0x10) = param_4;
  }
  *(undefined4 *)((int)this + 0x24) = 0x42b40000;
  *(undefined4 *)((int)this + 0x28) = 0x42b40000;
  *(undefined4 *)((int)this + 0x20) = 0;
  if ((undefined4 *)((int)this + 0x14) != &param_2) {
    *(undefined4 *)((int)this + 0x14) = 0;
    *(undefined4 *)((int)this + 0x18) = 0;
    *(undefined4 *)((int)this + 0x1c) = 0;
  }
  return this;
}

