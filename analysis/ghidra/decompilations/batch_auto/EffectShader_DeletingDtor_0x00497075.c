
void * __thiscall EffectShader_DeletingDtor(void *this,byte param_1)

{
  EffectShader_Dtor();
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

