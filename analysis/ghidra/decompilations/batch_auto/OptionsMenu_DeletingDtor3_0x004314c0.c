
void * __thiscall OptionsMenu_DeletingDtor3(void *this,byte param_1)

{
  OptionsMenu_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

