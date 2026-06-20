
void * __thiscall MeshNode_ctor(void *this,undefined4 param_1,char *param_2)

{
  *(undefined4 *)((int)this + 4) = param_1;
  *(undefined ***)this = &PTR_SpriteAnim_DeletingDtor_004d9c48;
  *(undefined1 *)((int)this + 0xc) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined1 *)((int)this + 0xe) = 0;
  *(undefined4 *)((int)this + 0x10) = 0;
  *(undefined1 *)((int)this + 0xd) = 0;
  LoadMesh(this,param_2,'\x01');
  return this;
}

