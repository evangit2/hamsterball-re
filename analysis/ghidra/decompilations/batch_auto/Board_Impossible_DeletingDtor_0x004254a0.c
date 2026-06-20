
void * __thiscall Board_Impossible_DeletingDtor(void *this,byte param_1)

{
  Board_Impossible_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

