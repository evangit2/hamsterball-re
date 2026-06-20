
void * __thiscall D3DXObject_ScalarDtor(void *this,byte param_1)

{
  D3DX_AssemblyOp8(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

