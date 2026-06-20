
void * __thiscall MusicChannel_Ctor(void *this,undefined4 param_1)

{
  *(undefined ***)this = &PTR_MusicChannel_DeletingDtor_004d91d8;
  AthenaList_Init((void *)((int)this + 0xc),0);
  *(undefined4 *)((int)this + 4) = param_1;
  *(undefined4 *)((int)this + 0x528) = 0x3f800000;
  *(undefined1 *)((int)this + 0x530) = 0;
  *(undefined1 *)((int)this + 0x531) = 0;
  return this;
}

