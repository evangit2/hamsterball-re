
void * __thiscall TourneyBoard_MenuScalarDtor(void *this,byte param_1)

{
  RumbleBoard_Menu_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

