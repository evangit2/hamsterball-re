
void __thiscall AthenaList_SortedInsert(void *this,int param_1)

{
  int iVar1;
  bool bVar2;
  void *pvVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;
  int iVar7;
  int *piVar8;
  undefined4 *puVar9;
  int *piVar10;
  int iVar11;
  
  if (*(int *)((int)this + 4) == 0) {
    *(undefined4 *)((int)this + 4) = 1;
    pvVar3 = _malloc(4);
    *(void **)((int)this + 0x40c) = pvVar3;
    puVar9 = (undefined4 *)((int)this + 8);
    for (iVar7 = 0x100; iVar7 != 0; iVar7 = iVar7 + -1) {
      *puVar9 = 0;
      puVar9 = puVar9 + 1;
    }
    **(int **)((int)this + 0x40c) = param_1;
    return;
  }
  piVar10 = *(int **)((int)this + 0x40c);
  iVar7 = *(int *)((int)this + 0x410);
  iVar4 = *(int *)((int)this + 4) + 1;
  *(int *)((int)this + 4) = iVar4;
  bVar2 = false;
  piVar5 = _malloc(iVar4 * 4);
  iVar4 = 0;
  piVar6 = piVar5;
  if (*(int *)((int)this + 4) != 1 && -1 < *(int *)((int)this + 4) + -1) {
    do {
      iVar11 = *(int *)(*(int *)((int)this + 0x410) + *piVar10);
      iVar1 = *(int *)(iVar7 + param_1);
      if (*(int *)((int)this + 0x414) == 1) {
        if ((iVar1 < iVar11) && (!bVar2)) {
          piVar8 = (int *)((int)this + 0xc);
          iVar11 = 0x20;
          do {
            if (iVar4 < piVar8[-1]) {
              piVar8[-1] = piVar8[-1] + 1;
            }
            if (iVar4 < *piVar8) {
              *piVar8 = *piVar8 + 1;
            }
            if (iVar4 < piVar8[1]) {
              piVar8[1] = piVar8[1] + 1;
            }
            if (iVar4 < piVar8[2]) {
              piVar8[2] = piVar8[2] + 1;
            }
            if (iVar4 < piVar8[3]) {
              piVar8[3] = piVar8[3] + 1;
            }
            if (iVar4 < piVar8[4]) {
              piVar8[4] = piVar8[4] + 1;
            }
            if (iVar4 < piVar8[5]) {
              piVar8[5] = piVar8[5] + 1;
            }
            if (iVar4 < piVar8[6]) {
              piVar8[6] = piVar8[6] + 1;
            }
            piVar8 = piVar8 + 8;
            iVar11 = iVar11 + -1;
          } while (iVar11 != 0);
LAB_00453471:
          bVar2 = true;
          *piVar6 = param_1;
          piVar6 = piVar6 + 1;
        }
      }
      else if ((iVar11 < iVar1) && (!bVar2)) {
        piVar8 = (int *)((int)this + 0xc);
        iVar11 = 0x20;
        do {
          if (iVar4 < piVar8[-1]) {
            piVar8[-1] = piVar8[-1] + 1;
          }
          if (iVar4 < *piVar8) {
            *piVar8 = *piVar8 + 1;
          }
          if (iVar4 < piVar8[1]) {
            piVar8[1] = piVar8[1] + 1;
          }
          if (iVar4 < piVar8[2]) {
            piVar8[2] = piVar8[2] + 1;
          }
          if (iVar4 < piVar8[3]) {
            piVar8[3] = piVar8[3] + 1;
          }
          if (iVar4 < piVar8[4]) {
            piVar8[4] = piVar8[4] + 1;
          }
          if (iVar4 < piVar8[5]) {
            piVar8[5] = piVar8[5] + 1;
          }
          if (iVar4 < piVar8[6]) {
            piVar8[6] = piVar8[6] + 1;
          }
          piVar8 = piVar8 + 8;
          iVar11 = iVar11 + -1;
        } while (iVar11 != 0);
        goto LAB_00453471;
      }
      *piVar6 = *piVar10;
      piVar10 = piVar10 + 1;
      piVar6 = piVar6 + 1;
      iVar4 = iVar4 + 1;
    } while (iVar4 < *(int *)((int)this + 4) + -1);
    if (bVar2) goto LAB_004534a7;
  }
  *piVar6 = param_1;
LAB_004534a7:
  _free(*(void **)((int)this + 0x40c));
  *(int **)((int)this + 0x40c) = piVar5;
  return;
}

