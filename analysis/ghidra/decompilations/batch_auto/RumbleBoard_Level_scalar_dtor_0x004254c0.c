
void * __thiscall ArenaBoard_Level_ScalarDtor(void *this,byte param_1)

{
  ArenaBoard_Impossible_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

