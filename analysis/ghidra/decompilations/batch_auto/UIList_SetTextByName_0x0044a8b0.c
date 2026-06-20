
void __thiscall UIList_SetTextByName(void *this,char *param_1,char *param_2)

{
  char cVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  char *pcVar5;
  undefined4 *puVar6;
  
  iVar2 = AthenaList_NextIndex((int)this + 0x88c);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x894) = 0;
  if (*(int *)((int)this + 0x890) < 1) {
    puVar6 = (undefined4 *)0x0;
  }
  else {
    puVar6 = (undefined4 *)**(undefined4 **)((int)this + 0xc98);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x894) = 1;
  }
  while( true ) {
    if (puVar6 == (undefined4 *)0x0) {
      return;
    }
    if (((char *)puVar6[1] != (char *)0x0) &&
       (iVar3 = __stricmp((char *)puVar6[1],param_2), iVar3 == 0)) {
      _free((void *)*puVar6);
      pcVar4 = param_1;
      do {
        cVar1 = *pcVar4;
        pcVar4 = pcVar4 + 1;
      } while (cVar1 != '\0');
      pcVar5 = operator_new((uint)(pcVar4 + (1 - (int)(param_1 + 1))));
      *puVar6 = pcVar5;
      pcVar4 = param_1;
      do {
        cVar1 = *pcVar4;
        pcVar4 = pcVar4 + 1;
        *pcVar5 = cVar1;
        pcVar5 = pcVar5 + 1;
      } while (cVar1 != '\0');
    }
    iVar3 = *(int *)((int)this + iVar2 * 4 + 0x894);
    if (*(int *)((int)this + 0x890) <= iVar3) break;
    puVar6 = *(undefined4 **)(*(int *)((int)this + 0xc98) + iVar3 * 4);
    *(int *)((int)this + iVar2 * 4 + 0x894) = iVar3 + 1;
  }
  return;
}

