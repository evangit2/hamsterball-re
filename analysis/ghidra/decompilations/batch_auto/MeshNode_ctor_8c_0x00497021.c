
void * __thiscall MeshNode_ctor_8c(void *this,undefined4 *param_1)

{
  Mesh_InitTexture(this,param_1,8,1);
  *(undefined ***)this = &PTR_MeshNode_DeletingDtor_004dc1d0;
  return this;
}

