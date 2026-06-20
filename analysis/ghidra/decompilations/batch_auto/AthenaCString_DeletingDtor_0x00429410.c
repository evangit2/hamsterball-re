
void * __thiscall AthenaCString_DeletingDtor(void *this,byte param_1)

{
  AthenaString_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

