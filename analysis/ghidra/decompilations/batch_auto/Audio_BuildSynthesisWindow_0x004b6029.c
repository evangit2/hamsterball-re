
void Audio_BuildSynthesisWindow(void)

{
  byte *pbVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;
  int *piVar7;
  int unaff_EDI;
  undefined1 *local_c;
  int local_8;
  
  iVar2 = *(int *)(unaff_EDI + 0x1a4);
  local_8 = 0;
  if (0 < *(int *)(unaff_EDI + 100)) {
    piVar6 = (int *)(iVar2 + 0x34);
    do {
      iVar3 = piVar6[-5];
      iVar4 = 0;
      if (0 < local_8) {
        piVar5 = (int *)(iVar2 + 0x20);
        do {
          if (iVar3 == *piVar5) {
            piVar5 = *(int **)(iVar2 + 0x34 + iVar4 * 4);
            if (piVar5 != (int *)0x0) goto LAB_004b60d3;
            break;
          }
          iVar4 = iVar4 + 1;
          piVar5 = piVar5 + 1;
        } while (iVar4 < local_8);
      }
      piVar5 = (int *)(*(code *)**(undefined4 **)(unaff_EDI + 4))();
      local_c = &DAT_004e92e0;
      piVar7 = piVar5;
      do {
        iVar4 = 0;
        do {
          pbVar1 = local_c + iVar4;
          iVar4 = iVar4 + 1;
          *piVar7 = (int)((uint)*pbVar1 * -0x1fe + 0xfe01) / (iVar3 * 0x200 + -0x200);
          piVar7 = piVar7 + 1;
        } while (iVar4 < 0x10);
        local_c = local_c + 0x10;
      } while ((int)local_c < 0x4e93e0);
LAB_004b60d3:
      local_8 = local_8 + 1;
      *piVar6 = (int)piVar5;
      piVar6 = piVar6 + 1;
    } while (local_8 < *(int *)(unaff_EDI + 100));
  }
  return;
}

