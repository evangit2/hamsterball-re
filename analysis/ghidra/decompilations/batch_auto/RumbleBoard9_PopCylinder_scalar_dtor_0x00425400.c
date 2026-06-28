
void * __thiscall ArenaBoard9_PopCylinder_ScalarDtor(void *this,byte param_1)

{
  ArenaBoard_Sky_DeletingDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

