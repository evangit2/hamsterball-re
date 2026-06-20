
int __cdecl StreamWriter_ReserveSpace(int param_1,size_t param_2)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  undefined4 *puVar4;
  void *pvVar5;
  
  piVar1 = *(int **)(param_1 + 4);
  if (piVar1 == (int *)0x0) {
    puVar4 = (undefined4 *)Pool_AllocateBufferNode();
    *(undefined4 **)(param_1 + 8) = puVar4;
    *(undefined4 **)(param_1 + 4) = puVar4;
    return *(int *)*puVar4;
  }
  iVar2 = piVar1[2];
  piVar3 = (int *)*piVar1;
  if ((int)param_2 <= (piVar3[1] - piVar1[1]) - iVar2) {
    return *piVar3 + piVar1[1] + iVar2;
  }
  if (iVar2 == 0) {
    if (piVar3[1] < (int)param_2) {
      pvVar5 = _realloc((void *)*piVar3,param_2);
      *piVar3 = (int)pvVar5;
      piVar3[1] = param_2;
    }
    return (*(undefined4 **)(param_1 + 4))[1] + *(int *)**(undefined4 **)(param_1 + 4);
  }
  puVar4 = (undefined4 *)Pool_AllocateBufferNode();
  *(undefined4 **)(*(int *)(param_1 + 4) + 0xc) = puVar4;
  *(undefined4 **)(param_1 + 4) = puVar4;
  return *(int *)*puVar4;
}

