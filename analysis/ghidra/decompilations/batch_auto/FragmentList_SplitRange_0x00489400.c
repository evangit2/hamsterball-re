
int * __fastcall FragmentList_SplitRange(int param_1,int *param_2)

{
  int iVar1;
  int in_EAX;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int *local_4;
  
  local_4 = (int *)0x0;
  for (; piVar2 = (int *)0x0, piVar3 = (int *)0x0, param_2 != (int *)0x0;
      param_2 = (int *)param_2[3]) {
    piVar4 = (int *)0x0;
    if (param_1 < param_2[2]) goto LAB_00489432;
    param_1 = param_1 - param_2[2];
  }
  goto joined_r0x004894a2;
  while( true ) {
    iVar1 = *(int *)(*param_2 + 0xc);
    *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) + 1;
    piVar2 = *(int **)(iVar1 + 4);
    if (piVar2 == (int *)0x0) {
      piVar2 = _malloc(0x10);
    }
    else {
      *(int *)(iVar1 + 4) = piVar2[3];
    }
    piVar2[1] = 0;
    piVar2[2] = 0;
    piVar2[3] = 0;
    piVar3 = piVar2;
    if (piVar4 != (int *)0x0) {
      piVar4[3] = (int)piVar2;
      piVar3 = local_4;
    }
    local_4 = piVar3;
    *piVar2 = *param_2;
    piVar2[1] = param_2[1] + param_1;
    piVar2[2] = in_EAX;
    if (param_2[2] - param_1 < in_EAX) {
      piVar2[2] = param_2[2] - param_1;
    }
    param_2 = (int *)param_2[3];
    param_1 = 0;
    in_EAX = in_EAX - piVar2[2];
    piVar4 = piVar2;
    piVar2 = local_4;
    piVar3 = local_4;
    if (param_2 == (int *)0x0) break;
LAB_00489432:
    piVar2 = local_4;
    piVar3 = local_4;
    if (in_EAX == 0) break;
  }
joined_r0x004894a2:
  for (; piVar2 != (int *)0x0; piVar2 = (int *)piVar2[3]) {
    *(int *)(*piVar2 + 8) = *(int *)(*piVar2 + 8) + 1;
  }
  return piVar3;
}

