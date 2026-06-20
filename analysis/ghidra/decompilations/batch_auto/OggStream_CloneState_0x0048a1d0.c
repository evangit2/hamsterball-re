
void __cdecl OggStream_CloneState(undefined4 *param_1,undefined4 *param_2)

{
  int *piVar1;
  
  param_1[1] = param_2[1];
  param_1[3] = param_2[3];
  piVar1 = FragmentList_CloneAll((int *)*param_2);
  *param_1 = piVar1;
  piVar1 = FragmentList_CloneAll((int *)param_2[2]);
  param_1[2] = piVar1;
  return;
}

