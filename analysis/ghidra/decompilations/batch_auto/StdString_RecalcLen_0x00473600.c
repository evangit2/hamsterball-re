
int __fastcall StdString_RecalcLen(int param_1)

{
  char *pcVar1;
  char cVar2;
  char *pcVar3;
  
  if (*(char *)(param_1 + 0x10) != '\0') {
    pcVar3 = *(char **)(param_1 + 4);
    if (pcVar3 != (char *)0x0) {
      pcVar1 = pcVar3 + 1;
      do {
        cVar2 = *pcVar3;
        pcVar3 = pcVar3 + 1;
      } while (cVar2 != '\0');
      *(int *)(param_1 + 0x14) = (int)pcVar3 - (int)pcVar1;
      *(undefined1 *)(param_1 + 0x10) = 0;
      return (int)pcVar3 - (int)pcVar1;
    }
    *(undefined4 *)(param_1 + 0x14) = 0;
    *(undefined1 *)(param_1 + 0x10) = 0;
  }
  return *(int *)(param_1 + 0x14);
}

