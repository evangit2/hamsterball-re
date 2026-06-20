
void __fastcall OptionsMenu_UpdateControl3(void *param_1)

{
  int iVar1;
  char local_400 [1024];
  
  iVar1 = *(int *)(*(int *)((int)param_1 + 0x878) + 0xb30);
  if (iVar1 == 99) {
    AthenaString_SprintfToBuffer(local_400,(byte *)"PLAYER 3: COMPUTER");
  }
  else if (iVar1 == 100) {
    AthenaString_SprintfToBuffer(local_400,(byte *)"PLAYER 3: OFF");
  }
  else {
    AthenaString_SprintfToBuffer(local_400,(byte *)"PLAYER 3: %s");
  }
  UIList_SetTextByName(param_1,local_400,"CONTROL3");
  OptionsMenu_RenderControls(param_1);
  return;
}

