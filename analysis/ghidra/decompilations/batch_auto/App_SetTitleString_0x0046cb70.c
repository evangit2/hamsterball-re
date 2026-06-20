
void __thiscall App_SetTitleString(void *this,char *param_1)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  
  _free(*(void **)((int)this + 0x1b4));
  *(undefined4 *)((int)this + 0x1b4) = 0;
  if (param_1 != (char *)0x0) {
    pcVar2 = operator_new(1);
    if (pcVar2 == (char *)0x0) {
      pcVar2 = (char *)0x0;
    }
    else {
      pcVar3 = param_1;
      do {
        cVar1 = *pcVar3;
        pcVar3 = pcVar3 + 1;
      } while (cVar1 != '\0');
      *pcVar2 = ((char)pcVar3 - ((char)param_1 + '\x01')) + '\x01';
    }
    *(char **)((int)this + 0x1b4) = pcVar2;
    do {
      cVar1 = *param_1;
      param_1 = param_1 + 1;
      *pcVar2 = cVar1;
      pcVar2 = pcVar2 + 1;
    } while (cVar1 != '\0');
  }
  return;
}

