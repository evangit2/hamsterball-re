
void * __thiscall TextureCache_DeletingDtor(void *this,byte param_1)

{
  TextureCache_RecursiveFree((int)this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

