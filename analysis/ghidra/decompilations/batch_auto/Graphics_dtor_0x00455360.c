
void * __thiscall Graphics_dtor(void *this,byte param_1)

{
  Graphics_Cleanup(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

