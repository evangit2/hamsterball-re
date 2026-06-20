
void * __thiscall MeshTexture_InitVolume(void *this,undefined4 *param_1)

{
  Mesh_InitTexture(this,param_1,0x10,1);
  *(undefined ***)this = &PTR_MeshNode_DeletingDtor_004dc1c0;
  return this;
}

