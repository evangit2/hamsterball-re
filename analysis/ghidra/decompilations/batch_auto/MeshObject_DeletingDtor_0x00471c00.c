
void * __thiscall MeshObject_DeletingDtor(void *this,byte param_1)

{
  MeshObject_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

