
void * __thiscall Level_DeletingDtor2(void *this,byte param_1)

{
  Level_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

