
void * __thiscall RaceGoalReached_ScalarDtor(void *this,byte param_1)

{
  RaceResults_Reset(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

