
void * __thiscall ArenaBoard_DeletingDtor(void *this,byte param_1)

{
  ArenaBoard_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

