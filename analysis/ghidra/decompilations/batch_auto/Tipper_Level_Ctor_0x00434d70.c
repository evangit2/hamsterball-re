
void * __thiscall
Tipper_Level_Ctor(void *this,int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  Level_ctor(this,*(undefined4 *)(*(int *)(param_1 + 0x878) + 0x174));
  *(undefined ***)this = &PTR_Tipper_DeletingDtor_004d5330;
  *(int *)((int)this + 0x10d0) = param_1;
  if ((undefined4 *)((int)this + 0x10d4) != &param_2) {
    *(undefined4 *)((int)this + 0x10d4) = param_2;
    *(undefined4 *)((int)this + 0x10d8) = param_3;
    *(undefined4 *)((int)this + 0x10dc) = param_4;
  }
  *(undefined1 *)(*(int *)((int)this + 0x480) + 0x10d0) = 1;
  return this;
}

