
void __fastcall OptionsMenu_UpdateControl1(void *param_1)

{
  char local_400 [1024];
  
  AthenaString_SprintfToBuffer(local_400,(byte *)"PLAYER 1: %s");
  UIList_SetTextByName(param_1,local_400,"CONTROL1");
  OptionsMenu_RenderControls(param_1);
  return;
}

