
void * __thiscall Rotator_DeletingDtor5(void *this,byte param_1)

{
  Rotator_Cleanup_vtable4(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

