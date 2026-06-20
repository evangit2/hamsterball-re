
void __fastcall Path_ComputeSegmentLengths(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  float local_8;
  int local_4;
  
  iVar6 = 0;
  if (*(void **)(param_1 + 0x24) != (void *)0x0) {
    _free(*(void **)(param_1 + 0x24));
  }
  *(undefined4 *)(param_1 + 0x24) = 0;
  *(undefined4 *)(param_1 + 0x28) = 0;
  *(undefined4 *)(param_1 + 0x2c) = 0;
  if (*(int *)(param_1 + 4) == 0) {
    iVar5 = 0;
  }
  else {
    iVar5 = *(int *)(param_1 + 8) - *(int *)(param_1 + 4) >> 2;
  }
  iVar5 = iVar5 + -1;
  local_4 = iVar5;
  if (0 < iVar5) {
    do {
      iVar1 = *(int *)(param_1 + 0x24);
      fVar4 = *(float *)(*(int *)(param_1 + 0x14) + 4 + iVar6 * 4) -
              *(float *)(*(int *)(param_1 + 0x14) + iVar6 * 4);
      fVar3 = *(float *)(*(int *)(param_1 + 4) + 4 + iVar6 * 4) -
              *(float *)(*(int *)(param_1 + 4) + iVar6 * 4);
      local_8 = SQRT(fVar4 * fVar4 + fVar3 * fVar3);
      if ((iVar1 == 0) ||
         ((uint)(*(int *)(param_1 + 0x2c) - iVar1 >> 2) <=
          (uint)(*(int *)(param_1 + 0x28) - iVar1 >> 2))) {
        Vector_InsertN((void *)(param_1 + 0x20),*(undefined4 **)(param_1 + 0x28),(undefined4 *)0x1,
                       &local_8);
      }
      else {
        puVar2 = *(undefined4 **)(param_1 + 0x28);
        Array_FillDWords(puVar2,1,&local_8);
        *(undefined4 **)(param_1 + 0x28) = puVar2 + 1;
        iVar5 = local_4;
      }
      iVar6 = iVar6 + 1;
    } while (iVar6 < iVar5);
  }
  return;
}

