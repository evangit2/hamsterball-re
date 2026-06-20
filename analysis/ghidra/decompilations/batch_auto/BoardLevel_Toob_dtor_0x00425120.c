
void * __thiscall BoardLevel_Toob_dtor(void *this,byte param_1)

{
  BoardLevel_Toob_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

