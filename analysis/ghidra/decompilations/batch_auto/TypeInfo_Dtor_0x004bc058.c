
void __fastcall TypeInfo_Dtor(undefined4 *param_1)

{
  *param_1 = &type_info::vftable;
  __lock(0xe);
  if ((void *)param_1[1] != (void *)0x0) {
    _free((void *)param_1[1]);
  }
  LeaveCriticalSection_indexed(0xe);
  return;
}

