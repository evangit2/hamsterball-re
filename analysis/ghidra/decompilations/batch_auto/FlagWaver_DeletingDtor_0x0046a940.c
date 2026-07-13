
void * __thiscall FlagWaver_DeletingDtor(void *this,byte param_1)

{
  FlagWaver_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

