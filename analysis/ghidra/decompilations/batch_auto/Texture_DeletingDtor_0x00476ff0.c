
void * __thiscall Texture_DeletingDtor(void *this,byte param_1)

{
  Texture_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

