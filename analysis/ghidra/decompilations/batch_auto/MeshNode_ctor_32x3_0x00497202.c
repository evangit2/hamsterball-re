
void * __thiscall MeshNode_ctor_32x3(void *this,undefined4 *param_1)

{
  Mesh_InitTexture(this,param_1,0x20,3);
  *(undefined ***)this = &PTR_MeshNode_DeletingDtor_004dc240;
  return this;
}

