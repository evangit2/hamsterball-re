
void * __thiscall KeyRemapMenu_DeletingDtor(void *this,byte param_1)

{
  KeyRemapMenu_Cleanup(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

