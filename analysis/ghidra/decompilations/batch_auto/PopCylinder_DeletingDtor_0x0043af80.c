
void * __thiscall PopCylinder_DeletingDtor(void *this,byte param_1)

{
  PopCylinder_Level_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

