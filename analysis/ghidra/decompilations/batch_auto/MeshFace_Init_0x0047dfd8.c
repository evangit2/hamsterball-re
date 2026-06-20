
void __thiscall MeshFace_Init(void *this,undefined4 param_1)

{
  *(undefined2 *)this = 0xffff;
  *(undefined2 *)((int)this + 2) = 0xffff;
  *(undefined2 *)((int)this + 4) = 0xffff;
  *(undefined1 *)((int)this + 0x10) = 0;
  *(undefined1 *)((int)this + 0x11) = 0;
  *(undefined4 *)((int)this + 0xc) = param_1;
  return;
}

