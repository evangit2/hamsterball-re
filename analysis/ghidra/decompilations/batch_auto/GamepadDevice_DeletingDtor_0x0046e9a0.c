
void * __thiscall GamepadDevice_DeletingDtor(void *this,byte param_1)

{
  int *piVar1;
  
  piVar1 = *(int **)((int)this + 8);
  *(undefined ***)this = &PTR_GamepadDevice_DeletingDtor_004d9844;
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    *(undefined4 *)((int)this + 8) = 0;
  }
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

