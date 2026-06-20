
void * __thiscall AthenaString_CopyCtor(void *this,int param_1)

{
  *(undefined ***)this = &PTR_AthenaCString_DeletingDtor_004d290c;
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  *(undefined1 *)((int)this + 0x18) = 1;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0x14) = 0;
  *(undefined1 *)((int)this + 0x10) = 0;
  AthenaString_AssignCStr(this,*(char **)(param_1 + 4));
  *(undefined1 *)((int)this + 0x18) = *(undefined1 *)(param_1 + 0x18);
  return this;
}

