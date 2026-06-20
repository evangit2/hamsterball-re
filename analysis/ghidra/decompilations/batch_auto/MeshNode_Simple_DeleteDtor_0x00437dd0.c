
void * __thiscall MeshNode_Simple_DeleteDtor(void *this,byte param_1)

{
  MeshNode_SimpleDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

