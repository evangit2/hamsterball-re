
void * __thiscall MeshWorld_DeletingDtor2(void *this,byte param_1)

{
  MeshWorld_dtor2(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

