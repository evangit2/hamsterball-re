
void * __thiscall Menu_DeletingDtor(void *this,byte param_1)

{
  Menu_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

