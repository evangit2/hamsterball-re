
void __cdecl FontFormatString_Parse(void *param_1,int param_2)

{
  char cVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined1 *local_104;
  char local_100;
  undefined1 auStack_ff [255];
  
  local_104 = &stack0x0000000c;
  _free(*(void **)((int)param_1 + 4));
  *(undefined4 *)((int)param_1 + 4) = 0;
  *(undefined4 *)((int)param_1 + 0x14) = 0;
  *(undefined4 *)((int)param_1 + 8) = 0;
  iVar3 = 0;
  iVar4 = iVar3;
  while( true ) {
    while (*(char *)(iVar3 + param_2) == '%') {
      StdString_AppendN(param_1,(char *)(iVar4 + param_2),iVar3 - iVar4);
      iVar3 = iVar3 + 1;
      iVar4 = iVar3;
      while( true ) {
        cVar1 = *(char *)(iVar4 + param_2);
        iVar2 = _isdigit((int)cVar1);
        if ((((iVar2 == 0) && (cVar1 != '.')) && (cVar1 != '-')) &&
           (((cVar1 != '+' && (cVar1 != '#')) && (cVar1 != ' ')))) break;
        iVar4 = iVar4 + 1;
      }
      _strncpy(&local_100,(char *)(iVar3 + param_2),(iVar4 - iVar3) + 2);
      auStack_ff[iVar4 - iVar3] = 0;
      CRT_ParseFormatString(&PTR_PTR_004f7820,&local_100,param_1,(int *)&local_104);
      iVar3 = iVar4 + 1;
      iVar4 = iVar3;
    }
    if (*(char *)(iVar3 + param_2) == '\0') break;
    iVar3 = iVar3 + 1;
  }
  StdString_AppendN(param_1,(char *)(iVar4 + param_2),iVar3 - iVar4);
  return;
}

