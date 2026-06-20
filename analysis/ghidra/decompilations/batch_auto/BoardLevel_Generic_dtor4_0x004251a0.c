
void * __thiscall BoardLevel_Generic_dtor4(void *this,byte param_1)

{
  BoardLevel_Master_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

