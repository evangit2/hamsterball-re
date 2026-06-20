
void * __thiscall BoardLevel5_Tower_dtor(void *this,byte param_1)

{
  BoardLevel5_Tower_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

