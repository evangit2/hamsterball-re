
void * __thiscall SpatialTree_DeletingDtor(void *this,byte param_1)

{
  SpatialTree_Free(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

