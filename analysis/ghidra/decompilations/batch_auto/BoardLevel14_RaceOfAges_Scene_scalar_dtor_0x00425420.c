
void * __thiscall BoardLevel14_RaceOfAges_Scene_scalar_dtor(void *this,byte param_1)

{
  Board_NeonRace_DeletingDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

