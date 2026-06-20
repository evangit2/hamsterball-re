
void * __thiscall SoundBuffer_ScalarDtor(void *this,byte param_1)

{
  *(undefined ***)this = &PTR_SoundBuffer_ScalarDtor_004d9124;
  _free(*(void **)((int)this + 4));
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

