
void * __thiscall OptionsMenu_DeletingDtor4(void *this,byte param_1)

{
  TimeTrialMenu_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

