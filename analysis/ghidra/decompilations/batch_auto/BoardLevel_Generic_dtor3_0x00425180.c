
void * __thiscall BoardLevel_Generic_dtor3(void *this,byte param_1)

{
  BoardLevel_Up_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

