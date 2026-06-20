
void * __thiscall GameObject2_DeletingDtor(void *this,byte param_1)

{
  GameObject2_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

