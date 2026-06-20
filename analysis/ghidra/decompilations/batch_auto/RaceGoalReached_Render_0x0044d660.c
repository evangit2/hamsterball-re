
void * __thiscall RaceGoalReached_Render(void *this,byte param_1)

{
  UIListItem_vtbl_Init(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

