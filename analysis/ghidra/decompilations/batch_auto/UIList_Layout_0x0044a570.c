
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall UIList_Layout(int *param_1)

{
  int iVar1;
  float fVar2;
  float fVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  undefined4 *puVar8;
  int *piVar9;
  int iVar10;
  ulonglong uVar11;
  
  *(undefined1 *)(param_1 + 0x32f) = 0;
  AthenaList_Free((int)(param_1 + 0x113));
  param_1[0x32c] = 0;
  iVar4 = AthenaList_NextIndex((int)(param_1 + 0x223));
  param_1[iVar4 + 0x225] = 0;
  if (param_1[0x224] < 1) {
    puVar8 = (undefined4 *)0x0;
  }
  else {
    puVar8 = *(undefined4 **)param_1[0x326];
    param_1[iVar4 + 0x225] = 1;
  }
  while (puVar8 != (undefined4 *)0x0) {
    if ((char *)*puVar8 != (char *)0x0) {
      uVar11 = Font_MeasureText((char *)*puVar8);
      iVar10 = (int)uVar11 + 2;
      if (puVar8[8] != 0) {
        iVar10 = (int)uVar11 + 7 + *(int *)(puVar8[8] + 200);
      }
      iVar5 = AthenaList_GetSize((int)(puVar8 + 10));
      if (iVar5 != 0) {
        iVar10 = iVar10 + 5;
      }
      iVar6 = AthenaList_NextIndex((int)(puVar8 + 10));
      iVar5 = 0;
      puVar8[iVar6 + 0xc] = 0;
      if (0 < (int)puVar8[0xb]) {
        iVar5 = *(int *)puVar8[0x10d];
        puVar8[iVar6 + 0xc] = 1;
      }
      while (iVar5 != 0) {
        iVar10 = iVar10 + *(int *)(iVar5 + 200);
        iVar7 = puVar8[iVar6 + 0xc];
        if ((int)puVar8[0xb] <= iVar7) break;
        iVar5 = *(int *)(puVar8[0x10d] + iVar7 * 4);
        puVar8[iVar6 + 0xc] = iVar7 + 1;
      }
      iVar5 = param_1[0x32c];
      iVar6 = iVar5;
      if (iVar5 <= iVar10) {
        iVar6 = iVar10;
      }
      iVar7 = param_1[0x32b];
      if ((iVar7 <= iVar6) && (iVar7 = iVar5, iVar5 <= iVar10)) {
        iVar7 = iVar10;
      }
      param_1[0x32c] = iVar7;
    }
    iVar10 = param_1[iVar4 + 0x225];
    if (param_1[0x224] <= iVar10) break;
    puVar8 = *(undefined4 **)(param_1[0x326] + iVar10 * 4);
    param_1[iVar4 + 0x225] = iVar10 + 1;
  }
  uVar11 = __ftol2();
  iVar10 = (int)uVar11;
  uVar11 = __ftol2();
  iVar4 = (int)uVar11;
  param_1[0x330] = 0;
  iVar5 = AthenaList_NextIndex((int)(param_1 + 0x223));
  param_1[iVar5 + 0x225] = 0;
  if (param_1[0x224] < 1) {
    piVar9 = (int *)0x0;
  }
  else {
    piVar9 = *(int **)param_1[0x326];
    param_1[iVar5 + 0x225] = 1;
  }
  while (piVar9 != (int *)0x0) {
    if (param_1[0x330] == 0) {
      param_1[0x330] = (int)piVar9;
    }
    iVar6 = param_1[0x21f];
    if (*(char *)((int)piVar9 + 0x441) != '\0') {
      iVar6 = param_1[0x220];
    }
    if ((char)piVar9[0x110] == '\0') {
      if (*piVar9 != 0) {
        iVar7 = *(int *)(iVar6 + 0x424);
        iVar1 = piVar9[7];
        fVar2 = (float)iVar7 * _DAT_004d6ad8;
        iVar6 = param_1[0x221];
        *(float *)(iVar1 + 0xc) = (float)param_1[0x32c];
        fVar3 = (float)_DAT_004cf438;
        *(float *)(iVar1 + 4) = (float)iVar10;
        *(float *)(iVar1 + 0x10) = (float)iVar7 * fVar3;
        *(float *)(iVar1 + 8) = fVar2 + (float)iVar6 + (float)iVar4;
        iVar6 = piVar9[7];
        goto LAB_0044a81a;
      }
    }
    else {
      iVar1 = piVar9[7];
      fVar2 = (float)*(int *)(iVar6 + 0x424) * _DAT_004d6ad8;
      iVar6 = param_1[0x221];
      iVar7 = piVar9[9];
      *(float *)(iVar1 + 0xc) = (float)param_1[0x32c];
      *(float *)(iVar1 + 4) = (float)iVar10;
      *(float *)(iVar1 + 0x10) = (float)iVar7;
      *(float *)(iVar1 + 8) = fVar2 + (float)iVar6 + (float)iVar4;
      iVar6 = piVar9[7];
LAB_0044a81a:
      AthenaList_Append(param_1 + 0x113,iVar6);
    }
    iVar4 = iVar4 + piVar9[9];
    iVar6 = param_1[iVar5 + 0x225];
    if (param_1[0x224] <= iVar6) break;
    piVar9 = *(int **)(param_1[0x326] + iVar6 * 4);
    param_1[iVar5 + 0x225] = iVar6 + 1;
  }
  iVar4 = param_1[0x32a];
  *(undefined4 *)(iVar4 + 8) = 0x41000000;
  *(undefined4 *)(iVar4 + 0xc) = 0x42240000;
  *(undefined4 *)(iVar4 + 0x10) = 0x42e40000;
  *(float *)(iVar4 + 4) = (float)(iVar10 + -0x37);
  iVar4 = param_1[0x329];
  *(undefined4 *)(iVar4 + 0x10) = 0x42e40000;
  *(float *)(iVar4 + 4) = (float)(iVar10 + -0x37);
  *(undefined4 *)(iVar4 + 8) = 0x43ed8000;
  *(undefined4 *)(iVar4 + 0xc) = 0x42240000;
  (**(code **)(*param_1 + 0x44))(1);
  return;
}

