
void * __thiscall Lifter_DeletingDtor(void *this,byte param_1)

{
  Lifter_Level_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

