
void __fastcall RegKey_SetSoftwarePath(int param_1)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  char local_104 [260];
  
  if (*(void **)(param_1 + 8) != (void *)0x0) {
    _free(*(void **)(param_1 + 8));
    *(undefined4 *)(param_1 + 8) = 0;
  }
  AthenaString_SprintfToBuffer(local_104,(byte *)"SOFTWARE\\%s");
  pcVar2 = local_104;
  do {
    cVar1 = *pcVar2;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  pcVar3 = _malloc((size_t)(pcVar2 + (1 - (int)(local_104 + 1))));
  *(char **)(param_1 + 8) = pcVar3;
  pcVar2 = local_104;
  do {
    cVar1 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    *pcVar3 = cVar1;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  return;
}

