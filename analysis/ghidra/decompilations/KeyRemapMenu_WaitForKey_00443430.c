// KeyRemapMenu_WaitForKey @ 0x00443430
// Decompiled via GhidraMCP v5.12.0-headless
// Session 2928 - Batch decompilation

void __fastcall KeyRemapMenu_WaitForKey(int *param_1)

{
  byte *pbVar1;
  int iVar2;
  
  if ((char)param_1[0x338] == '\0') {
    UIList_ScrollUpdate(param_1);
    return;
  }
  iVar2 = 0;
  if ((char)param_1[0x33a] != '\0') {
    *(undefined1 *)(param_1 + 0x33a) = 0;
    pbVar1 = (byte *)(*(int *)(*(int *)(param_1[0x21e] + 0x180) + 0x434) + 0xc);
    do {
      if ((*pbVar1 & 0x80) != 0) {
        *(undefined1 *)(param_1 + 0x33a) = 1;
        return;
      }
      iVar2 = iVar2 + 1;
      pbVar1 = pbVar1 + 1;
    } while (iVar2 < 0x100);
    return;
  }
  pbVar1 = (byte *)(*(int *)(*(int *)(param_1[0x21e] + 0x180) + 0x434) + 0xc);
  do {
    if ((*pbVar1 & 0x80) != 0) {
      *(int *)param_1[0x339] = iVar2;
      *(undefined1 *)(param_1 + 0x338) = 0;
      KeyRemapMenu_UpdateKeyLabels(param_1);
      *(undefined4 *)(param_1[0x21e] + 0x560) = 0x32;
      return;
    }
    iVar2 = iVar2 + 1;
    pbVar1 = pbVar1 + 1;
  } while (iVar2 < 0x100);
  return;
}