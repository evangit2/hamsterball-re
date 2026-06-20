
void * __thiscall MeshBuffer_DeletingDtor(void *this,byte param_1)

{
  MeshBuffer_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

