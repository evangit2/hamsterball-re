
int * __cdecl AthenaList_SplitChunk(int *param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  int *unaff_EDI;
  
  piVar1 = (int *)*unaff_EDI;
  piVar2 = piVar1;
  while( true ) {
    if (piVar2 == (int *)0x0) {
      return (int *)0x0;
    }
    piVar5 = piVar2 + 2;
    if (param_2 <= *piVar5) break;
    piVar2 = (int *)piVar2[3];
    param_2 = param_2 - *piVar5;
  }
  if (param_2 == 0) {
    return (int *)0x0;
  }
  iVar3 = piVar2[2];
  if (param_2 < iVar3) {
    iVar4 = piVar2[1];
    piVar5 = (int *)Pool_AllocateNode(*(int *)(*piVar2 + 0xc));
    *unaff_EDI = (int)piVar5;
    *piVar5 = *piVar2;
    *(int *)(*unaff_EDI + 4) = iVar4 + param_2;
    *(int *)(*unaff_EDI + 8) = iVar3 - param_2;
    *(int *)(*unaff_EDI + 0xc) = piVar2[3];
    *(int *)(*(int *)*unaff_EDI + 8) = *(int *)(*(int *)*unaff_EDI + 8) + 1;
    if ((param_1 != (int *)0x0) && (piVar2 == (int *)*param_1)) {
      *param_1 = *unaff_EDI;
    }
    piVar2[2] = param_2;
    piVar2[3] = 0;
    return piVar1;
  }
  if (piVar2[3] != 0) {
    *unaff_EDI = piVar2[3];
    piVar2[3] = 0;
    return piVar1;
  }
  *param_1 = 0;
  *unaff_EDI = 0;
  return piVar1;
}

