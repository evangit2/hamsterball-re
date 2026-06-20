
void * __thiscall GameObject2_scalar_dtor(void *this,byte param_1)

{
  QuitAbortDialog_ScalarDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

