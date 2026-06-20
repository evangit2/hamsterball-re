
void __thiscall AthenaString_AssignCStr(void *this,char *param_1)

{
  char *pcVar1;
  char cVar2;
  char *pcVar3;
  uint uVar4;
  char *pcVar5;
  
  _free(*(void **)((int)this + 4));
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 0x14) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined1 *)((int)this + 0x18) = 0;
  if (param_1 == (char *)0x0) {
    param_1 = "";
    *(undefined1 *)((int)this + 0x18) = 1;
  }
  pcVar1 = param_1 + 1;
  pcVar3 = param_1;
  do {
    cVar2 = *pcVar3;
    pcVar3 = pcVar3 + 1;
  } while (cVar2 != '\0');
  uVar4 = ((int)pcVar3 - (int)pcVar1) + 1;
  uVar4 = uVar4 & ((int)uVar4 < 1) - 1;
  *(uint *)((int)this + 8) = uVar4;
  pcVar5 = operator_new(uVar4);
  *(char **)((int)this + 4) = pcVar5;
  do {
    cVar2 = *param_1;
    param_1 = param_1 + 1;
    *pcVar5 = cVar2;
    pcVar5 = pcVar5 + 1;
  } while (cVar2 != '\0');
  *(int *)((int)this + 0x14) = (int)pcVar3 - (int)pcVar1;
  return;
}

