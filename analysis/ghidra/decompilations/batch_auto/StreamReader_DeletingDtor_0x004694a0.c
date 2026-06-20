
void * __thiscall StreamReader_DeletingDtor(void *this,byte param_1)

{
  StreamReader_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

