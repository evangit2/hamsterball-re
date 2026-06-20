
void * __thiscall MeshNode_ctor_16bx2(void *this,undefined4 *param_1)

{
  Mesh_InitTexture(this,param_1,0x10,2);
  *(undefined ***)this = &PTR_MeshNode_DeletingDtor_004dc220;
  return this;
}

