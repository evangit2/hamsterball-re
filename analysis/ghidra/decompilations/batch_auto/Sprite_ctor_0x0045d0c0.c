
void * __thiscall Sprite_ctor(void *this,void *param_1,char *param_2)

{
  undefined4 uVar1;
  void *pvVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cce1b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined ***)this = &PTR_Sprite_ScalarDtor_004d8f84;
  RenderContext_Init((undefined4 *)((int)this + 8));
  *(undefined4 *)((int)this + 0x68) = 0xffffffff;
  *(undefined4 *)((int)this + 0x84) = 0xffffffff;
  *(undefined4 *)((int)this + 0xa0) = 0xffffffff;
  *(undefined4 *)((int)this + 0xbc) = 0xffffffff;
  local_4 = 0;
  *(void **)((int)this + 4) = param_1;
  *(undefined1 *)((int)this + 0xd0) = 1;
  *(undefined4 *)((int)this + 0x60) = 0x3f800000;
  *(undefined4 *)((int)this + 100) = 0x3f800000;
  *(undefined4 *)((int)this + 0x6c) = 0;
  *(undefined4 *)((int)this + 0x70) = 0;
  *(undefined4 *)((int)this + 0x7c) = 0x3f800000;
  *(undefined4 *)((int)this + 0x80) = 0x3f800000;
  *(undefined4 *)((int)this + 0x88) = 0x3f800000;
  *(undefined4 *)((int)this + 0x8c) = 0;
  *(undefined4 *)((int)this + 0x98) = 0x3f800000;
  *(undefined4 *)((int)this + 0x9c) = 0x3f800000;
  *(undefined4 *)((int)this + 0xa4) = 0;
  *(undefined4 *)((int)this + 0xa8) = 0x3f800000;
  *(undefined4 *)((int)this + 0xb4) = 0x3f800000;
  *(undefined4 *)((int)this + 0xb8) = 0x3f800000;
  *(undefined4 *)((int)this + 0xc0) = 0x3f800000;
  *(undefined4 *)((int)this + 0xc4) = 0x3f800000;
  pvVar2 = Graphics_FindOrCreateTexture(param_1,param_2,'\x01');
  *(void **)((int)this + 0x50) = pvVar2;
  *(undefined4 *)((int)this + 0xc) = 0x3f800000;
  *(undefined4 *)((int)this + 0x10) = 0x3f800000;
  *(undefined4 *)((int)this + 0x14) = 0x3f800000;
  *(undefined4 *)((int)this + 0x18) = 0x3f800000;
  *(undefined1 *)((int)this + 0x54) = 0;
  *(undefined4 *)((int)this + 200) = *(undefined4 *)(*(int *)((int)this + 0x50) + 0x14);
  uVar1 = *(undefined4 *)(*(int *)((int)this + 0x50) + 0x18);
  *(undefined1 *)((int)this + 0xd1) = 0;
  *(undefined4 *)((int)this + 0xcc) = uVar1;
  ExceptionList = local_c;
  return this;
}

