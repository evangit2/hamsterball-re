
void * __thiscall TourneyContinueDialog_scalar_dtor(void *this,byte param_1)

{
  TourneyContinueDialog_ScalarDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

