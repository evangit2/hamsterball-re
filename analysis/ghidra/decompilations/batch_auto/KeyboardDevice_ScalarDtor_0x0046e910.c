
void * __thiscall KeyboardDevice_ScalarDtor(void *this,byte param_1)

{
  KeyboardDevice_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

