
void __fastcall Sound_Play3DAtPosition(int param_1)

{
  int iVar1;
  
  iVar1 = Audio_ClampPanValue();
  (**(code **)(**(int **)(param_1 + 4) + 0x3c))(*(int **)(param_1 + 4),iVar1);
  return;
}

