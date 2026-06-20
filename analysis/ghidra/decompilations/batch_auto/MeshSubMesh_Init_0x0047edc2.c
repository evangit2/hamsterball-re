
void __thiscall
MeshSubMesh_Init(void *this,undefined4 param_1,undefined4 param_2,undefined2 param_3)

{
  *(undefined4 *)((int)this + 0x10) = 0;
  *(undefined4 *)((int)this + 0x20) = 0;
  *(undefined4 *)this = param_1;
  *(undefined4 *)((int)this + 4) = param_2;
  *(undefined2 *)((int)this + 0x18) = param_3;
  return;
}

