
void * __thiscall TimerArray_Ctor(void *this,undefined4 param_1)

{
  *(undefined ***)this = &PTR_TimerArray_DeletingDtor_004da190;
  _eh_vector_constructor_iterator_((void *)((int)this + 8),0x44,2,Timer_Init,Timer_Cleanup);
  *(undefined4 *)((int)this + 4) = param_1;
  return this;
}

