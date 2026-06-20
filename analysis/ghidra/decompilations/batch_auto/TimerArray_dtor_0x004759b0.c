
void __fastcall TimerArray_dtor(undefined4 *param_1)

{
  *param_1 = &PTR_TimerArray_DeletingDtor_004da190;
  _eh_vector_destructor_iterator_(param_1 + 2,0x44,2,Timer_Cleanup);
  return;
}

