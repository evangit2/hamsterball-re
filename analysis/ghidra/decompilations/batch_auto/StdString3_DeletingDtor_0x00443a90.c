
void * __thiscall StdString3_DeletingDtor(void *this,byte param_1)

{
  StdString3_Clear(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

