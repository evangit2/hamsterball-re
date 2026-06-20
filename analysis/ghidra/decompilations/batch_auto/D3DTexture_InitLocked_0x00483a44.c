
void * __thiscall D3DTexture_InitLocked(void *this,int *param_1,uint param_2,uint param_3)

{
  undefined4 uVar1;
  
  *(undefined ***)this = &PTR_D3DDevice_Reset_004db3d0;
  *(uint *)((int)this + 4) = param_2;
  *(uint *)((int)this + 8) = param_3;
  *(undefined4 *)((int)this + 0xc) = 4;
  *(undefined4 *)((int)this + 0x20) = 0;
  *(undefined4 *)((int)this + 0x24) = 0;
  *(undefined4 *)((int)this + 0x28) = 0;
  uVar1 = VertexDecl_CreateFromFVF(param_2);
  *(undefined4 *)((int)this + 0x2c) = uVar1;
  *(int **)((int)this + 0x24) = param_1;
  *(undefined4 *)((int)this + 0x30) = 0;
  *(undefined4 *)((int)this + 0x34) = 0;
  *(undefined4 *)((int)this + 0x38) = 0;
  *(undefined4 *)((int)this + 0x3c) = 0;
  *(undefined4 *)((int)this + 0x40) = 0;
  *(undefined4 *)((int)this + 0x44) = 0;
  *(undefined4 *)((int)this + 0x48) = 0;
  *(undefined4 *)((int)this + 0x4c) = 0;
  *(undefined4 *)((int)this + 0x50) = 0;
  *(undefined4 *)((int)this + 0x54) = 0;
  *(undefined4 *)((int)this + 0x58) = 0;
  *(undefined4 *)((int)this + 0x5c) = 0;
  *(undefined4 *)((int)this + 0x60) = 1;
  *(undefined4 *)((int)this + 100) = 0;
  *(undefined4 *)((int)this + 0x10) = 0;
  *(undefined4 *)((int)this + 0x18) = 0;
  if ((param_3 & 0x200) != 0) {
    *(undefined4 *)((int)this + 0x10) = 1;
  }
  if ((param_3 & 0x100) != 0) {
    *(undefined4 *)((int)this + 0x10) = 2;
  }
  if ((param_3 & 0x400) != 0) {
    *(undefined4 *)((int)this + 0x18) = 8;
  }
  if ((param_3 & 2) != 0) {
    *(uint *)((int)this + 0x18) = *(uint *)((int)this + 0x18) | 0x20;
  }
  if ((param_3 & 4) != 0) {
    *(uint *)((int)this + 0x18) = *(uint *)((int)this + 0x18) | 0x40;
  }
  if ((param_3 & 8) != 0) {
    *(byte *)((int)this + 0x18) = *(byte *)((int)this + 0x18) | 0x80;
  }
  if ((param_3 & 0x4000) != 0) {
    *(byte *)((int)this + 0x19) = *(byte *)((int)this + 0x19) | 1;
  }
  if ((param_3 & 0x800) != 0) {
    *(uint *)((int)this + 0x18) = *(uint *)((int)this + 0x18) | 0x200;
  }
  if ((param_3 & 0x10000) != 0) {
    *(uint *)((int)this + 0x18) = *(uint *)((int)this + 0x18) | 0x10;
  }
  *(undefined4 *)((int)this + 0x14) = 0;
  *(undefined4 *)((int)this + 0x1c) = 0;
  if ((param_3 & 0x20) != 0) {
    *(undefined4 *)((int)this + 0x14) = 1;
  }
  if ((param_3 & 0x10) != 0) {
    *(undefined4 *)((int)this + 0x14) = 2;
  }
  if ((param_3 & 0x40) != 0) {
    *(undefined4 *)((int)this + 0x1c) = 8;
  }
  if ((param_3 & 2) != 0) {
    *(uint *)((int)this + 0x1c) = *(uint *)((int)this + 0x1c) | 0x20;
  }
  if ((param_3 & 4) != 0) {
    *(uint *)((int)this + 0x1c) = *(uint *)((int)this + 0x1c) | 0x40;
  }
  if ((param_3 & 8) != 0) {
    *(byte *)((int)this + 0x1c) = *(byte *)((int)this + 0x1c) | 0x80;
  }
  if ((param_3 & 0x4000) != 0) {
    *(byte *)((int)this + 0x1d) = *(byte *)((int)this + 0x1d) | 1;
  }
  if ((char)param_3 < '\0') {
    *(uint *)((int)this + 0x1c) = *(uint *)((int)this + 0x1c) | 0x200;
  }
  if ((char)(param_3 >> 8) < '\0') {
    *(uint *)((int)this + 0x1c) = *(uint *)((int)this + 0x1c) | 0x10;
  }
  if ((param_3 & 0x1000) != 0) {
    *(undefined4 *)((int)this + 0xc) = 0x24;
  }
  (**(code **)(*param_1 + 4))(param_1);
  (**(code **)(**(int **)((int)this + 0x24) + 0x18))(*(int **)((int)this + 0x24),(int)this + 0x20);
  return this;
}

