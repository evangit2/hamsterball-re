
void __thiscall StdString_InsertCStr(void *this,uint param_1,char *param_2)

{
  char cVar1;
  char *pcVar2;
  
  pcVar2 = param_2;
  do {
    cVar1 = *pcVar2;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  StdString_Insert(this,param_1,(undefined4 *)param_2,(int)pcVar2 - (int)(param_2 + 1));
  return;
}

