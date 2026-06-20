
void * __thiscall MeshNode_ctor_64(void *this,undefined4 *param_1)

{
  Mesh_InitTexture(this,param_1,0x40,1);
  *(undefined ***)this = &PTR_MeshNode_DeletingDtor_004dc2c0;
  return this;
}

