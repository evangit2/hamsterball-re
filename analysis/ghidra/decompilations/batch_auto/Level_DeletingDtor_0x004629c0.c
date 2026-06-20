
void * __thiscall Level_DeletingDtor(void *this,byte param_1)

{
  Level_Cleanup(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

