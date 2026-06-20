
void __cdecl PNG_DispatchRow(int *param_1,int *param_2)

{
  if ((*(byte *)(param_1 + 0x17) & 0x40) == 0) {
    PNG_SetupRowTransform(param_1);
  }
  PNG_ApplyTransforms((int)param_1,param_2);
  return;
}

