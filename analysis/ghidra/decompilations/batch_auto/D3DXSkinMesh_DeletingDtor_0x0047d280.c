
void * __thiscall D3DXSkinMesh_DeletingDtor(void *this,byte param_1)

{
  Mesh_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

