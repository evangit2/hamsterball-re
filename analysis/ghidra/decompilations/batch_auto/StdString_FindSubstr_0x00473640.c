
int __thiscall StdString_FindSubstr(void *this,char *param_1)

{
  char *pcVar1;
  
  if (param_1 != (char *)0x0) {
    pcVar1 = strstr(*(char **)((int)this + 4),param_1);
    if (pcVar1 != (char *)0x0) {
      return (int)pcVar1 - *(int *)((int)this + 4);
    }
  }
  return -1;
}

