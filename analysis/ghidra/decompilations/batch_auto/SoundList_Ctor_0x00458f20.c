
void * __thiscall SoundList_Ctor(void *this,undefined4 param_1)

{
  *(undefined ***)this = &PTR_SoundList_ScalarDtor_004d8e7c;
  AthenaList_Init((void *)((int)this + 8),0);
  *(undefined4 *)((int)this + 4) = param_1;
  return this;
}

