
int * __cdecl FragmentList_CloneAll(int *param_1)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int *piVar5;
  
  piVar2 = (int *)0x0;
  piVar3 = (int *)0x0;
  for (; piVar4 = piVar2, param_1 != (int *)0x0; param_1 = (int *)param_1[3]) {
    iVar1 = *(int *)(*param_1 + 0xc);
    *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) + 1;
    piVar4 = *(int **)(iVar1 + 4);
    if (piVar4 == (int *)0x0) {
      piVar4 = _malloc(0x10);
    }
    else {
      *(int *)(iVar1 + 4) = piVar4[3];
    }
    piVar4[1] = 0;
    piVar4[2] = 0;
    piVar4[3] = 0;
    piVar5 = piVar4;
    if (piVar3 != (int *)0x0) {
      piVar3[3] = (int)piVar4;
      piVar5 = piVar2;
    }
    *piVar4 = *param_1;
    piVar4[1] = param_1[1];
    piVar4[2] = param_1[2];
    piVar2 = piVar5;
    piVar3 = piVar4;
  }
  for (; piVar4 != (int *)0x0; piVar4 = (int *)piVar4[3]) {
    *(int *)(*piVar4 + 8) = *(int *)(*piVar4 + 8) + 1;
  }
  return piVar2;
}

