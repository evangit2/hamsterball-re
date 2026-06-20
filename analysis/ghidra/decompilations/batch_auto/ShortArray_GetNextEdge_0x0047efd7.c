
uint * ShortArray_GetNextEdge(uint *param_1,ushort *param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  
  uVar2 = (uint)*(ushort *)
                 (*(int *)(param_3 + 4) +
                 ((*(int *)(param_2 + 2) + 2U) % 3 + (uint)*param_2 * 3) * 2);
  uVar1 = 0xffff;
  if (uVar2 != 0xffff) {
    uVar1 = ShortArray_FindMatch3(*(int *)(param_3 + 4) + uVar2 * 6,*param_2);
  }
  *param_1 = uVar2;
  param_1[1] = uVar1;
  return param_1;
}

