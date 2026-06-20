
void * __thiscall Scene_ScalarDtorBase(void *this,byte param_1)

{
  *(undefined ***)this = &PTR_Scene_ScalarDtorBase_004d8e6c;
  _free(*(void **)((int)this + 0x10));
  *(undefined4 *)((int)this + 0x10) = 0;
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

