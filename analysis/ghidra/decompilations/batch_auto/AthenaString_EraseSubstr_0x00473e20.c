
void __thiscall AthenaString_EraseSubstr(void *this,char *param_1)

{
  char cVar1;
  char *pcVar2;
  size_t sVar3;
  
  if (param_1 != (char *)0x0) {
    pcVar2 = strstr(*(char **)((int)this + 4),param_1);
    if ((pcVar2 != (char *)0x0) &&
       (sVar3 = (int)pcVar2 - *(int *)((int)this + 4), sVar3 != 0xffffffff)) {
      pcVar2 = param_1 + 1;
      do {
        cVar1 = *param_1;
        param_1 = param_1 + 1;
      } while (cVar1 != '\0');
      AthenaString_EraseRange(this,sVar3,(int)param_1 - (int)pcVar2);
    }
  }
  return;
}

