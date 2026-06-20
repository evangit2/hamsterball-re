
void __fastcall OptionsMenu_ApplySettings(int param_1)

{
  int iVar1;
  void *this;
  bool bVar2;
  
  iVar1 = *(int *)(param_1 + 0x878);
  bVar2 = false;
  if ((*(int *)(iVar1 + 0x15c) != *(int *)(param_1 + 0xce4)) ||
     (*(int *)(iVar1 + 0x160) != *(int *)(param_1 + 0xce8))) {
    *(int *)(iVar1 + 0x15c) = *(int *)(param_1 + 0xce4);
    *(undefined4 *)(*(int *)(param_1 + 0x878) + 0x160) = *(undefined4 *)(param_1 + 0xce8);
    bVar2 = true;
  }
  if (*(char *)(*(int *)(param_1 + 0x878) + 0x158) != *(char *)(param_1 + 0xce1)) {
    *(char *)(*(int *)(param_1 + 0x878) + 0x158) = *(char *)(param_1 + 0xce1);
    bVar2 = true;
  }
  if (*(char *)(*(int *)(param_1 + 0x878) + 0x238) != *(char *)(param_1 + 0xce0)) {
    *(char *)(*(int *)(param_1 + 0x878) + 0x238) = *(char *)(param_1 + 0xce0);
  }
  iVar1 = *(int *)(*(int *)(param_1 + 0x878) + 0x174);
  if (*(char *)(iVar1 + 0x18c) != *(char *)(param_1 + 0xcec)) {
    *(char *)(iVar1 + 0x18c) = *(char *)(param_1 + 0xcec);
    bVar2 = true;
  }
  iVar1 = *(int *)(*(int *)(param_1 + 0x878) + 0x174);
  if (*(char *)(iVar1 + 0x7d7) == *(char *)(param_1 + 0xced)) {
    if (!bVar2) {
      return;
    }
  }
  else {
    *(char *)(iVar1 + 0x7d7) = *(char *)(param_1 + 0xced);
    this = *(void **)(*(int *)(param_1 + 0x878) + 0x174);
    Graphics_SetViewportZ
              (this,*(undefined4 *)((int)this + 0x73c),*(undefined4 *)((int)this + 0x740));
  }
  App_SetFullScreen(*(void **)(param_1 + 0x878),*(char *)((int)*(void **)(param_1 + 0x878) + 0x158))
  ;
  return;
}

