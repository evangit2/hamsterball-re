
void __fastcall OptionsMenu_UpdateControl4(void *param_1)

{
  int iVar1;
  char local_400 [1024];
  
  iVar1 = *(int *)(*(int *)((int)param_1 + 0x878) + 0xb34);
  if (iVar1 == 99) {
    AthenaString_SprintfToBuffer(local_400,(byte *)"PLAYER 4: COMPUTER");
  }
  else if (iVar1 == 100) {
    AthenaString_SprintfToBuffer(local_400,(byte *)"PLAYER 4: OFF");
  }
  else {
    AthenaString_SprintfToBuffer(local_400,(byte *)"PLAYER 4: %s");
  }
  UIList_SetTextByName(param_1,local_400,"CONTROL4");
  OptionsMenu_RenderControls(param_1);
  return;
}

