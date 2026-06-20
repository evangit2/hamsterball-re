
void * __thiscall
Sprite_Ctor(void *this,undefined4 param_1,int param_2,int param_3,int param_4,int param_5,
           int param_6)

{
  int iVar1;
  float fVar2;
  float fVar3;
  
  *(undefined ***)this = &PTR_Sprite_ScalarDtor_004d8f84;
  RenderContext_Init((undefined4 *)((int)this + 8));
  *(undefined4 *)((int)this + 4) = param_1;
  *(undefined1 *)((int)this + 0xd0) = 0;
  fVar3 = (float)(param_3 + -1 + param_5);
  *(undefined4 *)((int)this + 0x60) = 0x3f800000;
  *(undefined4 *)((int)this + 100) = 0x3f800000;
  fVar2 = (float)(param_4 + -1 + param_6);
  *(undefined4 *)((int)this + 0x68) = 0xffffffff;
  *(float *)((int)this + 0x6c) = (float)param_3 / (float)*(int *)(param_2 + 0x14);
  iVar1 = *(int *)(param_2 + 0x18);
  *(undefined4 *)((int)this + 0x84) = 0xffffffff;
  *(undefined4 *)((int)this + 0x7c) = 0x3f800000;
  *(undefined4 *)((int)this + 0x80) = 0x3f800000;
  *(float *)((int)this + 0x70) = (float)param_4 / (float)iVar1;
  *(float *)((int)this + 0x88) = fVar3 / (float)*(int *)(param_2 + 0x14);
  iVar1 = *(int *)(param_2 + 0x18);
  *(undefined4 *)((int)this + 0xa0) = 0xffffffff;
  *(undefined4 *)((int)this + 0x98) = 0x3f800000;
  *(undefined4 *)((int)this + 0x9c) = 0x3f800000;
  *(float *)((int)this + 0x8c) = (float)param_4 / (float)iVar1;
  *(float *)((int)this + 0xa4) = (float)param_3 / (float)*(int *)(param_2 + 0x14);
  iVar1 = *(int *)(param_2 + 0x18);
  *(undefined4 *)((int)this + 0xbc) = 0xffffffff;
  *(undefined4 *)((int)this + 0xb4) = 0x3f800000;
  *(undefined4 *)((int)this + 0xb8) = 0x3f800000;
  *(float *)((int)this + 0xa8) = fVar2 / (float)iVar1;
  *(float *)((int)this + 0xc0) = fVar3 / (float)*(int *)(param_2 + 0x14);
  iVar1 = *(int *)(param_2 + 0x18);
  *(int *)((int)this + 0x50) = param_2;
  *(float *)((int)this + 0xc4) = fVar2 / (float)iVar1;
  *(undefined4 *)((int)this + 0xc) = 0x3f800000;
  *(undefined4 *)((int)this + 0x10) = 0x3f800000;
  *(undefined4 *)((int)this + 0x14) = 0x3f800000;
  *(undefined4 *)((int)this + 0x18) = 0x3f800000;
  *(undefined1 *)((int)this + 0x54) = 0;
  *(int *)((int)this + 0xcc) = param_6;
  *(int *)((int)this + 200) = param_5;
  *(undefined1 *)((int)this + 0xd1) = 0;
  return this;
}

