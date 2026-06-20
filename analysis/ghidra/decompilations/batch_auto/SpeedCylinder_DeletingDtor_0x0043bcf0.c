
void * __thiscall SpeedCylinder_DeletingDtor(void *this,byte param_1)

{
  SpeedCylinder_Level_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

