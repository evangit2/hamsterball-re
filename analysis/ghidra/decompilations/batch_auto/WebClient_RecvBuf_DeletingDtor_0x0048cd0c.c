
void * __thiscall WebClient_RecvBuf_DeletingDtor(void *this,byte param_1)

{
  WebClient_RecvBuf_Dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

