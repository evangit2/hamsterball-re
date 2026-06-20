
void * __thiscall Sprite_ScalarDtor(void *this,byte param_1)

{
  Sprite_Reset(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

