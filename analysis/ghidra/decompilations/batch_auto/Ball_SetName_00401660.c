/*
 * Function: Ball_SetName
 * Address: 0x00401660
 * Signature: void __thiscall Ball_SetName(void *this,char *param_1)
 *
 * Patterns: frees memory, allocates, ball. Calls: Ball_SetName, _free, operator_new. Offsets: 2, Lines: 24
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_SetName(void *this,char *param_1)

{
  char cVar1;
  char *pcVar2;
  
  if (*(void **)((int)this + 0xc28) != (void *)0x0) {
    _free(*(void **)((int)this + 0xc28));
  }
  pcVar2 = param_1;
  do {
    cVar1 = *pcVar2;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  pcVar2 = operator_new((uint)(pcVar2 + (1 - (int)(param_1 + 1))));
  *(char **)((int)this + 0xc28) = pcVar2;
  do {
    cVar1 = *param_1;
    param_1 = param_1 + 1;
    *pcVar2 = cVar1;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  *(undefined4 *)((int)this + 0xc) = 200;
  return;
}
