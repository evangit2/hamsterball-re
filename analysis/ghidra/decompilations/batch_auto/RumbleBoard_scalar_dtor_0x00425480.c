
void * __thiscall ArenaBoard_ScalarDtor(void *this,byte param_1)

{
  ArenaBoard_DeletingDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

