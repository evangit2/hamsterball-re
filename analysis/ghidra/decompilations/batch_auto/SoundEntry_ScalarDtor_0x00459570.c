
void * __thiscall SoundEntry_ScalarDtor(void *this,byte param_1)

{
  int *piVar1;
  
  piVar1 = *(int **)((int)this + 4);
  *(undefined ***)this = &PTR_SoundEntry_ScalarDtor_004d8e78;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    *(undefined4 *)((int)this + 4) = 0;
  }
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

