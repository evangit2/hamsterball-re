
void * __thiscall App_LoadOrSaveConfig_DeletingDtor(void *this,byte param_1)

{
  LoadOrSaveConfig(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

