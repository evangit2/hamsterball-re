
void * __thiscall Transform_ScalarDtor(void *this,byte param_1)

{
  D3DX_RegistryGetter_Ctor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

