
void * __thiscall Rotator_DeletingDtor7(void *this,byte param_1)

{
  Rotator_Cleanup_vtable6(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

