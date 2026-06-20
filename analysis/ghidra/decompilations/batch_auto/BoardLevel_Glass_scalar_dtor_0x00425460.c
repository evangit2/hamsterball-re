
void * __thiscall BoardLevel_Glass_scalar_dtor(void *this,byte param_1)

{
  BoardLevel_Glass_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

