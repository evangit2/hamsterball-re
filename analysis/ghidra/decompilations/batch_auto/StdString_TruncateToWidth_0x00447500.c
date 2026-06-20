
void StdString_TruncateToWidth(void *param_1)

{
  char *pcVar1;
  int iVar2;
  char *pcVar3;
  ulonglong uVar4;
  
  pcVar1 = (char *)((int)param_1 + 4);
  pcVar3 = pcVar1;
  if (0xf < *(uint *)((int)param_1 + 0x18)) {
    pcVar3 = *(char **)pcVar1;
  }
  uVar4 = Font_MeasureText(pcVar3);
  iVar2 = (int)uVar4;
  while (0x13f < iVar2) {
    StdString_Erase(param_1,*(int *)((int)param_1 + 0x14) - 1,1);
    pcVar3 = pcVar1;
    if (0xf < *(uint *)((int)param_1 + 0x18)) {
      pcVar3 = *(char **)pcVar1;
    }
    uVar4 = Font_MeasureText(pcVar3);
    iVar2 = (int)uVar4;
  }
  return;
}

