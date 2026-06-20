
void * __thiscall HighScoreEntry_ScalarDtor(void *this,byte param_1)

{
  ConfirmMenu_OnKey(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

