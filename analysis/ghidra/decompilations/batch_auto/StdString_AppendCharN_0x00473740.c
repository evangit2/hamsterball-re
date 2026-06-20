
void __thiscall StdString_AppendCharN(void *this,undefined1 param_1,int param_2)

{
  char *pcVar1;
  char cVar2;
  char *pcVar3;
  int iVar4;
  int iVar5;
  
  if (0 < param_2) {
    if (*(char *)((int)this + 0x18) == '\0') {
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
      iVar4 = *(int *)((int)this + 0x14);
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
      iVar5 = *(int *)((int)this + 0x14) + 1 + param_2;
    }
    else {
      iVar5 = param_2 + 1;
      iVar4 = 0;
    }
    StdString_Reserve(this,iVar5);
    iVar5 = iVar4 + param_2;
    for (; iVar4 < iVar5; iVar4 = iVar4 + 1) {
      *(undefined1 *)(iVar4 + *(int *)((int)this + 4)) = param_1;
    }
    *(undefined1 *)(iVar4 + *(int *)((int)this + 4)) = 0;
    *(int *)((int)this + 0x14) = *(int *)((int)this + 0x14) + param_2;
  }
  return;
}

