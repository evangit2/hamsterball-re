
void * __thiscall MeshNode_ScalarDtor(void *this,byte param_1)

{
  D3DXMesh_ComputeLightingThenAssemble();
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

