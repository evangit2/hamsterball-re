
void * __thiscall Ball_dtor(void *this,byte param_1)

{
  Ball_dtor2(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

