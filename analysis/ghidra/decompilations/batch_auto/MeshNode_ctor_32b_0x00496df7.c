
void * __thiscall MeshNode_ctor_32b(void *this,undefined4 *param_1)

{
  Mesh_InitTexture(this,param_1,0x20,1);
  *(undefined ***)this = &PTR_MeshNode_DeletingDtor_004dc1b0;
  return this;
}

