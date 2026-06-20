
void * __thiscall TimerArray_DeletingDtor(void *this,byte param_1)

{
  TimerArray_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

