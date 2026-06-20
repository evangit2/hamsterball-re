
void * __thiscall StdString_SubstrInit(void *this,void *param_1,uint param_2,uint param_3)

{
  *(undefined4 *)((int)param_1 + 0x14) = 0;
  *(undefined4 *)((int)param_1 + 0x18) = 0xf;
  *(undefined1 *)((int)param_1 + 4) = 0;
  StdString_Substr(param_1,this,param_2,param_3);
  return param_1;
}

