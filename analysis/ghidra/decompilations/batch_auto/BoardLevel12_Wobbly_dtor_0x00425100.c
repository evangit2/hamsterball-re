
void * __thiscall BoardLevel12_Wobbly_dtor(void *this,byte param_1)

{
  BoardLevel12_Wobbly_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

