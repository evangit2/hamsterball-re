
void * __thiscall RegKey_DeletingDtor(void *this,byte param_1)

{
  *(undefined ***)this = &PTR_RegKey_DeletingDtor_004d9f08;
  if (*(void **)((int)this + 8) != (void *)0x0) {
    _free(*(void **)((int)this + 8));
    *(undefined4 *)((int)this + 8) = 0;
  }
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

