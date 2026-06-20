
void * __thiscall BoardLevel8_Expert_dtor(void *this,byte param_1)

{
  BoardLevel8_Expert_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

