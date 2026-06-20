
void * __thiscall StreamReader_DeletingDtor_Close(void *this,byte param_1)

{
  *(undefined ***)this = &PTR_StreamReader_DeletingDtor_Close_004db2cc;
  __close(*(int *)((int)this + 4));
  if (*(void **)((int)this + 8) != (void *)0x0) {
    _free(*(void **)((int)this + 8));
    *(undefined4 *)((int)this + 8) = 0;
  }
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

