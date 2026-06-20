
void * __thiscall MeshTexture_InitCubeMap(void *this,undefined4 *param_1)

{
  Mesh_InitTexture(this,param_1,0x20,1);
  *(undefined ***)this = &PTR_MeshNode_DeletingDtor_004dc2a0;
  return this;
}

