
void * __thiscall RaptisoftUtil_DeletingDtor(void *this,byte param_1)

{
  *(undefined ***)this = &PTR_RaptisoftUtil_DeletingDtor_004d92ec;
  Window_Notify((int)this,(byte *)"*** END RAPTISOFT SESSION ***");
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

