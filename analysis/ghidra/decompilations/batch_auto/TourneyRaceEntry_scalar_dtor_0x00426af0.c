
void * __thiscall TourneyRaceEntry_scalar_dtor(void *this,byte param_1)

{
  *(undefined ***)this = &PTR_TourneyRaceEntry_scalar_dtor_004d2628;
  if (*(int **)((int)this + 0xc) != (int *)0x0) {
    (**(code **)(**(int **)((int)this + 0xc) + 0x40))();
    *(undefined4 *)((int)this + 0xc) = 0;
  }
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

