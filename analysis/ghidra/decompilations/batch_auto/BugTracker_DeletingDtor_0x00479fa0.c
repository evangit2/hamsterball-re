
void * __thiscall BugTracker_DeletingDtor(void *this,byte param_1)

{
  BugTracker_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

