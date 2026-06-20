
void * __thiscall D3DTexture_DeletingDtor(void *this,byte param_1)

{
  D3DTexture_NullDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

