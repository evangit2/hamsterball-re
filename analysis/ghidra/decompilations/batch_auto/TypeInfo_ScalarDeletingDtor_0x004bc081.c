
void * __thiscall TypeInfo_ScalarDeletingDtor(void *this,byte param_1)

{
  TypeInfo_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

