
void * __thiscall ArenaCollisionLevel_DeletingDtor(void *this,byte param_1)

{
  ArenaCollisionLevel_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

