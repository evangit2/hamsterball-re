
void * __thiscall Exception_ScalarDeletingDtor(void *this,byte param_1)

{
  Exception_VbaseDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

