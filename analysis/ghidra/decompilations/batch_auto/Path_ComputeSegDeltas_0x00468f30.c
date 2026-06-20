
void Path_ComputeSegDeltas(int param_1,void *param_2)

{
  float fVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  
  iVar3 = param_1;
  if ((*(int *)(param_1 + 4) != 0) &&
     (uVar7 = *(int *)(param_1 + 8) - *(int *)(param_1 + 4) >> 2, 1 < uVar7)) {
    iVar8 = uVar7 - 1;
    Vector_Resize(param_2,iVar8 * 4);
    iVar5 = 0;
    param_1 = 0;
    if (3 < iVar8) {
      iVar6 = 3;
      iVar4 = 0x1c;
      do {
        fVar1 = *(float *)(*(int *)(iVar3 + 4) + -0xc + iVar6 * 4);
        fVar2 = *(float *)(*(int *)(iVar3 + 4) + iVar6 * 4 + -8);
        *(undefined4 *)(iVar4 + -0x1c + *(int *)((int)param_2 + 4)) = 0;
        *(undefined4 *)(iVar4 + -0x18 + *(int *)((int)param_2 + 4)) = 0;
        *(float *)(iVar4 + -0x14 + *(int *)((int)param_2 + 4)) = fVar2 - fVar1;
        iVar6 = iVar6 + 4;
        *(float *)(iVar4 + -0x10 + *(int *)((int)param_2 + 4)) = fVar1;
        fVar1 = *(float *)(*(int *)(iVar3 + 4) + -0x18 + iVar6 * 4);
        fVar2 = *(float *)(*(int *)(iVar3 + 4) + -0x14 + iVar6 * 4);
        *(undefined4 *)(iVar4 + -0xc + *(int *)((int)param_2 + 4)) = 0;
        *(undefined4 *)(iVar4 + -8 + *(int *)((int)param_2 + 4)) = 0;
        *(float *)(iVar4 + -4 + *(int *)((int)param_2 + 4)) = fVar2 - fVar1;
        *(float *)(iVar4 + *(int *)((int)param_2 + 4)) = fVar1;
        fVar1 = *(float *)(*(int *)(iVar3 + 4) + -0x14 + iVar6 * 4);
        fVar2 = *(float *)(*(int *)(iVar3 + 4) + -0x10 + iVar6 * 4);
        *(undefined4 *)(iVar4 + 4 + *(int *)((int)param_2 + 4)) = 0;
        *(undefined4 *)(iVar4 + 8 + *(int *)((int)param_2 + 4)) = 0;
        *(float *)(iVar4 + 0xc + *(int *)((int)param_2 + 4)) = fVar2 - fVar1;
        *(float *)(iVar4 + 0x10 + *(int *)((int)param_2 + 4)) = fVar1;
        fVar1 = *(float *)(*(int *)(iVar3 + 4) + -0x10 + iVar6 * 4);
        fVar2 = *(float *)(*(int *)(iVar3 + 4) + -0xc + iVar6 * 4);
        *(undefined4 *)(iVar4 + 0x14 + *(int *)((int)param_2 + 4)) = 0;
        *(undefined4 *)(iVar4 + 0x18 + *(int *)((int)param_2 + 4)) = 0;
        *(float *)(iVar4 + 0x1c + *(int *)((int)param_2 + 4)) = fVar2 - fVar1;
        *(float *)(iVar4 + 0x20 + *(int *)((int)param_2 + 4)) = fVar1;
        iVar5 = param_1 + 4;
        iVar4 = iVar4 + 0x40;
        param_1 = iVar5;
      } while (iVar6 < iVar8);
    }
    if (iVar5 < iVar8) {
      iVar4 = iVar5 * 4;
      iVar8 = iVar8 - iVar5;
      do {
        fVar1 = *(float *)(*(int *)(iVar3 + 4) + iVar4);
        fVar2 = *(float *)(*(int *)(iVar3 + 4) + 4 + iVar4);
        *(undefined4 *)(*(int *)((int)param_2 + 4) + iVar4 * 4) = 0;
        *(undefined4 *)(*(int *)((int)param_2 + 4) + 4 + iVar4 * 4) = 0;
        *(float *)(*(int *)((int)param_2 + 4) + 8 + iVar4 * 4) = fVar2 - fVar1;
        iVar4 = iVar4 + 4;
        iVar8 = iVar8 + -1;
        *(float *)(*(int *)((int)param_2 + 4) + -4 + iVar4 * 4) = fVar1;
      } while (iVar8 != 0);
    }
  }
  return;
}

