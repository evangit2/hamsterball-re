
void * __thiscall Pendulum_scalar_dtor(void *this,byte param_1)

{
  Pendulum_Cleanup_vtable2(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

