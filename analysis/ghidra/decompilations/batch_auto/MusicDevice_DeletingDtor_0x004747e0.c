
void * __thiscall MusicDevice_DeletingDtor(void *this,byte param_1)

{
  MusicDevice_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

