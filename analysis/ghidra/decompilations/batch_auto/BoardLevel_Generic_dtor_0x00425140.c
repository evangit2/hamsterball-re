
void * __thiscall BoardLevel_Generic_dtor(void *this,byte param_1)

{
  BoardLevel_Sky_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

