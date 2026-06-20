
int * __thiscall D3DX_SkinWeights_VectorDtor(void *this,byte param_1)

{
  int *_Memory;
  
  if ((param_1 & 2) == 0) {
    _free(*(void **)this);
    _Memory = this;
    if ((param_1 & 1) != 0) {
      _free(this);
    }
  }
  else {
    _Memory = (int *)((int)this + -4);
    _eh_vector_destructor_iterator_(this,0xc,*_Memory,FUN_0048eb17);
    if ((param_1 & 1) != 0) {
      _free(_Memory);
    }
  }
  return _Memory;
}

