
void __thiscall AthenaString_ReplaceSubstr(void *this,char *param_1,char *param_2)

{
  char *pcVar1;
  size_t sVar2;
  
  if (param_1 != (char *)0x0) {
    pcVar1 = strstr(*(char **)((int)this + 4),param_1);
    if (pcVar1 == (char *)0x0) {
      sVar2 = 0xffffffff;
    }
    else {
      sVar2 = (int)pcVar1 - *(int *)((int)this + 4);
    }
    AthenaString_EraseSubstr(this,param_1);
    AthenaString_ReplaceAt(this,param_2,sVar2);
  }
  return;
}

