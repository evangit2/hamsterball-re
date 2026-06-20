
void * __thiscall Tower_Level_scalar_dtor(void *this,byte param_1)

{
  Tower_Level_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

