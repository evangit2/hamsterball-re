
void * __thiscall Blockdawg_DeletingDtor(void *this,byte param_1)

{
  Blockdawg_Level_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

