
void * __thiscall ScoreDisplay_CtorA(void *this,int param_1,byte *param_2,undefined4 param_3)

{
  RegisterDialog_ctor(this,param_1,param_2);
  *(undefined ***)this = &PTR_ScoreDisplay_DeletingDtor_004d6918;
  *(undefined4 *)((int)this + 0x115c) = param_3;
  return this;
}

