
void * __thiscall RumbleBoard_Level_scalar_dtor(void *this,byte param_1)

{
  RumbleBoard_Impossible_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

