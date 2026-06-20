
void __thiscall StdString_Reserve(void *this,int param_1)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  
  if ((param_1 != 0) && (*(int *)((int)this + 8) < param_1)) {
    *(uint *)((int)this + 8) = param_1 + 1U;
    pcVar2 = operator_new(param_1 + 1U);
    pcVar3 = *(char **)((int)this + 4);
    pcVar6 = pcVar2;
    if (pcVar3 != (char *)0x0) {
      do {
        cVar1 = *pcVar3;
        pcVar3 = pcVar3 + 1;
        *pcVar6 = cVar1;
        pcVar6 = pcVar6 + 1;
      } while (cVar1 != '\0');
      _free(*(void **)((int)this + 4));
      *(char **)((int)this + 4) = pcVar2;
      return;
    }
    uVar5 = *(uint *)((int)this + 8);
    pcVar3 = pcVar2;
    for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      pcVar3[0] = '\0';
      pcVar3[1] = '\0';
      pcVar3[2] = '\0';
      pcVar3[3] = '\0';
      pcVar3 = pcVar3 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar3 = '\0';
      pcVar3 = pcVar3 + 1;
    }
    _free(*(void **)((int)this + 4));
    *(char **)((int)this + 4) = pcVar2;
  }
  return;
}

