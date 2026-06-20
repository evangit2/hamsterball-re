
void * __thiscall SoundList_ScalarDtor(void *this,byte param_1)

{
  SoundList_DtorInner(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

