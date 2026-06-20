
void * __thiscall MusicChannel_DeletingDtor(void *this,byte param_1)

{
  MusicChannel_Cleanup(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

