
void * __thiscall MeshNode_RenderState_DeleteDtor(void *this,byte param_1)

{
  MeshNode_RenderStateDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

