
void * __thiscall RumbleBoard_Object_DeletingDtor(void *this,byte param_1)

{
  RumbleBoard_Object_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

