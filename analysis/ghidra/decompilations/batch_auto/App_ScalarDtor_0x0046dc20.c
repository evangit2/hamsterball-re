
void * __thiscall App_ScalarDtor(void *this,byte param_1)

{
  App_Shutdown(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

