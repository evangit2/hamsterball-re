
void * __thiscall MeshNode_ctor_16e(void *this,undefined4 *param_1)

{
  Mesh_InitTexture(this,param_1,0x10,1);
  *(undefined ***)this = &PTR_MeshNode_DeletingDtor_004dc190;
  return this;
}

