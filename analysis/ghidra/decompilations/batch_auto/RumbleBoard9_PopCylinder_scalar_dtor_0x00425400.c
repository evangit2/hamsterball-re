
void * __thiscall RumbleBoard9_PopCylinder_scalar_dtor(void *this,byte param_1)

{
  RumbleBoard_SkyArena_DeletingDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

