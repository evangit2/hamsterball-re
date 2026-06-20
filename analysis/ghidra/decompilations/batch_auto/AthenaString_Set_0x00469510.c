
void __thiscall AthenaString_Set(void *this,char *param_1)

{
  char cVar1;
  char *pcVar2;
  
  if (*(int *)((int)this + 0x10) != 0) {
    __close(*(int *)((int)this + 0x10));
  }
  _free(*(void **)((int)this + 8));
  *(undefined4 *)((int)this + 8) = 0;
  pcVar2 = param_1;
  do {
    cVar1 = *pcVar2;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  pcVar2 = operator_new((uint)(pcVar2 + (1 - (int)(param_1 + 1))));
  *(char **)((int)this + 8) = pcVar2;
  do {
    cVar1 = *param_1;
    param_1 = param_1 + 1;
    *pcVar2 = cVar1;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  *(undefined1 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 0xc) = *(undefined4 *)((int)this + 8);
  *(undefined1 *)((int)this + 0x14) = 0;
  return;
}

