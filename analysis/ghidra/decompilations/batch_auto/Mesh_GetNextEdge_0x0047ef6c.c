
void Mesh_GetNextEdge(int *param_1,int *param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)(*(int *)(param_3 + 4) + ((param_2[1] + 2U) % 3 + *param_2 * 3) * 4);
  if (iVar1 == -1) {
    iVar2 = -1;
  }
  else {
    iVar2 = FindInSmallIntArray(*(int *)(param_3 + 4) + iVar1 * 0xc,*param_2);
  }
  *param_1 = iVar1;
  param_1[1] = iVar2;
  return;
}

