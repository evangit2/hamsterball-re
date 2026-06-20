
void * __thiscall AthenaCString_ctor(void *this,char *param_1)

{
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0x14) = 0;
  *(undefined1 *)((int)this + 0x10) = 0;
  *(undefined ***)this = &PTR_AthenaCString_DeletingDtor_004d290c;
  *(undefined1 *)((int)this + 0x18) = 1;
  AthenaString_AssignCStr(this,param_1);
  return this;
}

