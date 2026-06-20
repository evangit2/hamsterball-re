
void __fastcall Scene_RenderIfVisible(int param_1)

{
  int iVar1;
  
  if (*(char *)(*(int *)(param_1 + 8) + 0x83c) != '\0') {
    iVar1 = Audio_ClampPanValue();
    (**(code **)(**(int **)(param_1 + 4) + 0x3c))(*(int **)(param_1 + 4),iVar1);
    (**(code **)(**(int **)(param_1 + 4) + 0x30))(*(int **)(param_1 + 4),0,0,1);
  }
  return;
}

