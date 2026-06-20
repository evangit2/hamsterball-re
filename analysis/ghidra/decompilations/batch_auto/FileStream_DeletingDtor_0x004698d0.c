
void * __thiscall FileStream_DeletingDtor(void *this,byte param_1)

{
  *(undefined ***)this = &PTR_FileStream_DeletingDtor_004d91bc;
  if (*(int *)((int)this + 0x10) != 0) {
    __close(*(int *)((int)this + 0x10));
  }
  _free(*(void **)((int)this + 8));
  *(undefined4 *)((int)this + 8) = 0;
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

