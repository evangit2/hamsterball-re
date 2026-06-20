
void * __thiscall OptionsMenu_DeletingDtor2(void *this,byte param_1)

{
  PauseMenu_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

