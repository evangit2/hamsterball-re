
void * __thiscall QuitToDesktop_scalar_dtor(void *this,byte param_1)

{
  QuitToDesktop_ScalarDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

