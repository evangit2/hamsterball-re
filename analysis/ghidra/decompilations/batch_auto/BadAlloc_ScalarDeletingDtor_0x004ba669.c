
void * __thiscall BadAlloc_ScalarDeletingDtor(void *this,byte param_1)

{
  BadAlloc_VbaseDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

