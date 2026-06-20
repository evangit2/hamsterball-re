
void * __thiscall BaseObject_DeletingDtor(void *this,byte param_1)

{
  BaseObject_Init(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

