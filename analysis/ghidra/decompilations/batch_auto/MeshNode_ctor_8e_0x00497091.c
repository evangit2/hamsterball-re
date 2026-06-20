
void * __thiscall MeshNode_ctor_8e(void *this,undefined4 *param_1)

{
  Mesh_InitTexture(this,param_1,8,1);
  *(undefined ***)this = &PTR_MeshNode_DeletingDtor_004dc200;
  return this;
}

