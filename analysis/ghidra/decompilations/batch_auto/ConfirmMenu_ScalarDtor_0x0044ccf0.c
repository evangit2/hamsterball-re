
void * __thiscall ConfirmMenu_ScalarDtor(void *this,byte param_1)

{
  RaceResults_DtorPhase1(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

