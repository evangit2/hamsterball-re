
void * __thiscall AthenaHashTable_DeletingDtor(void *this,byte param_1)

{
  AthenaHashTable_ctor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

