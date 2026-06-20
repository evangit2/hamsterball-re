
void __thiscall AthenaString_Assign(void *this,char *param_1)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  uint uVar4;
  char *pcVar5;
  char *pcVar6;
  
  if (param_1 == (char *)0x0) {
    param_1 = "";
  }
  if (*(char *)((int)this + 0x18) != '\0') {
    AthenaString_AssignCStr(this,param_1);
    return;
  }
  pcVar2 = param_1;
  do {
    cVar1 = *pcVar2;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  if (*(char *)((int)this + 0x10) != '\0') {
    pcVar3 = *(char **)((int)this + 4);
    if (pcVar3 == (char *)0x0) {
      *(undefined4 *)((int)this + 0x14) = 0;
    }
    else {
      pcVar6 = pcVar3 + 1;
      do {
        cVar1 = *pcVar3;
        pcVar3 = pcVar3 + 1;
      } while (cVar1 != '\0');
      *(int *)((int)this + 0x14) = (int)pcVar3 - (int)pcVar6;
    }
    *(undefined1 *)((int)this + 0x10) = 0;
  }
  StdString_Reserve(this,(int)(pcVar2 + ((*(int *)((int)this + 0x14) + 1) - (int)(param_1 + 1))));
  pcVar3 = param_1;
  do {
    cVar1 = *pcVar3;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  pcVar6 = (char *)(*(int *)((int)this + 4) + -1);
  do {
    pcVar5 = pcVar6 + 1;
    pcVar6 = pcVar6 + 1;
  } while (*pcVar5 != '\0');
  pcVar5 = param_1;
  for (uVar4 = (uint)((int)pcVar3 - (int)param_1) >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar4 = (int)pcVar3 - (int)param_1 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar6 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  }
  *(char **)((int)this + 0x14) = pcVar2 + (*(int *)((int)this + 0x14) - (int)(param_1 + 1));
  return;
}

