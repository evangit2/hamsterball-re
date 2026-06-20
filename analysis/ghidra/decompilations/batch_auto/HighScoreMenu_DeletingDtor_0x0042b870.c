
void * __thiscall HighScoreMenu_DeletingDtor(void *this,byte param_1)

{
  HighScoreMenu_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

