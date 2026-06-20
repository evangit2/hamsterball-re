
void * __thiscall Scene_ScalarDtor(void *this,byte param_1)

{
  Scene_Destroy(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

