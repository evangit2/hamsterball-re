
void __thiscall
AthenaHashTable_Lookup(void *this,undefined4 *param_1,char *param_2,undefined1 *param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  
  iVar2 = AthenaList_NextIndex(*(int *)((int)this + 0x480) + 0x894);
  iVar3 = *(int *)((int)this + 0x480);
  *(undefined4 *)(iVar3 + 0x89c + iVar2 * 4) = 0;
  if (*(int *)(iVar3 + 0x898) < 1) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    puVar4 = (undefined4 *)**(undefined4 **)(iVar3 + 0xca0);
    *(undefined4 *)(iVar3 + 0x89c + iVar2 * 4) = 1;
  }
  while (puVar4 != (undefined4 *)0x0) {
    iVar3 = __stricmp(param_2,(char *)*puVar4);
    if (iVar3 == 0) {
      if (param_3 != (undefined1 *)0x0) {
        *param_3 = 1;
      }
      if (param_1 == puVar4 + 1) {
        return;
      }
      *param_1 = puVar4[1];
      param_1[1] = puVar4[2];
      param_1[2] = puVar4[3];
      return;
    }
    iVar3 = *(int *)((int)this + 0x480);
    iVar1 = *(int *)(iVar3 + 0x89c + iVar2 * 4);
    if (*(int *)(iVar3 + 0x898) <= iVar1) break;
    puVar4 = *(undefined4 **)(*(int *)(iVar3 + 0xca0) + iVar1 * 4);
    *(int *)(iVar3 + 0x89c + iVar2 * 4) = iVar1 + 1;
  }
  if (param_3 != (undefined1 *)0x0) {
    *param_3 = 0;
  }
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  return;
}

