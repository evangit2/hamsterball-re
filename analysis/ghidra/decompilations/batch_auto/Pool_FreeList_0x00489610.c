
void __fastcall Pool_FreeList(int *param_1)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  
  while (param_1 != (int *)0x0) {
    iVar1 = *param_1;
    piVar2 = (int *)param_1[3];
    piVar3 = *(int **)(iVar1 + 0xc);
    iVar4 = *(int *)(iVar1 + 8) + -1;
    *(int *)(iVar1 + 8) = iVar4;
    if (iVar4 == 0) {
      piVar3[2] = piVar3[2] + -1;
      *(int *)(iVar1 + 0xc) = *piVar3;
      *piVar3 = iVar1;
    }
    piVar3[2] = piVar3[2] + -1;
    param_1[3] = piVar3[1];
    piVar3[1] = (int)param_1;
    AthenaList_FreeAllChunks(piVar3);
    param_1 = piVar2;
  }
  return;
}

