
void * __thiscall RumbleBoard_Odd_Arena_scalar_dtor(void *this,byte param_1)

{
  RumbleBoard_NeonArena_DeletingDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

