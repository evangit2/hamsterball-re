
void __fastcall WebClient_RecvBuf_Dtor(undefined4 *param_1)

{
  *param_1 = &PTR_COM_QueryInterface_004dbeac;
  if ((void *)param_1[3] != (void *)0x0) {
    _free((void *)param_1[3]);
  }
  return;
}

