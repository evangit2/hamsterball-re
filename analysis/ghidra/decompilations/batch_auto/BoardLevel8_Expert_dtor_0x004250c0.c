
void * __thiscall LevelBoard_Expert_dtor(void *this,byte param_1)

{
  LevelBoard_Expert_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

