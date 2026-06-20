
void __thiscall StdString_AppendN(void *this,char *param_1,size_t param_2)

{
  char *pcVar1;
  char cVar2;
  char *pcVar3;
  
  if (param_1 == (char *)0x0) {
    param_1 = "";
  }
  if (*(char *)((int)this + 0x18) != '\0') {
    StdString_AssignN(this,param_1,param_2);
    return;
  }
  if (*(char *)((int)this + 0x10) != '\0') {
    pcVar3 = *(char **)((int)this + 4);
    if (pcVar3 == (char *)0x0) {
      *(undefined4 *)((int)this + 0x14) = 0;
    }
    else {
      pcVar1 = pcVar3 + 1;
      do {
        cVar2 = *pcVar3;
        pcVar3 = pcVar3 + 1;
      } while (cVar2 != '\0');
      *(int *)((int)this + 0x14) = (int)pcVar3 - (int)pcVar1;
    }
    *(undefined1 *)((int)this + 0x10) = 0;
  }
  StdString_Reserve(this,*(int *)((int)this + 0x14) + 1 + param_2);
  _strncat(*(char **)((int)this + 4),param_1,param_2);
  *(size_t *)((int)this + 0x14) = *(int *)((int)this + 0x14) + param_2;
  return;
}

