
void * __thiscall MPMenu_DeletingDtor2(void *this,byte param_1)

{
  MPMenu_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

