
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */
/* WARNING: Unable to track spacebase fully for stack */

undefined4 __cdecl
Font_RenderChannels(int param_1,int *param_2,int param_3,int param_4,undefined *param_5)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  undefined1 *puVar9;
  undefined1 *puVar10;
  int *piVar11;
  int iVar12;
  int aiStack_4c [3];
  int local_24;
  int local_1c;
  int local_18;
  int local_10;
  int local_c;
  int local_8;
  
  piVar1 = (int *)*param_2;
  iVar2 = *(int *)param_2[5];
  iVar3 = piVar1[2];
  iVar6 = (piVar1[1] - *piVar1) / iVar3;
  aiStack_4c[2] = 0x4a921b;
  iVar5 = param_4 * -4;
  puVar10 = &stack0xffffffc0 + iVar5;
  iVar12 = 0;
  if (0 < param_4) {
    do {
      aiStack_4c[2 - param_4] = ((iVar6 + -1 + iVar2) / iVar2) * 4;
      aiStack_4c[1 - param_4] = param_1;
      aiStack_4c[-param_4] = 0x4a9240;
      iVar7 = Pool_Alloc(aiStack_4c[1 - param_4],aiStack_4c[2 - param_4]);
      *(int *)(&stack0xffffffc0 + iVar12 * 4 + iVar5) = iVar7;
      iVar12 = iVar12 + 1;
    } while (iVar12 < param_4);
  }
  local_c = 0;
  if (0 < param_2[3]) {
    do {
      iVar12 = 0;
      local_10 = 0;
      if (0 < iVar6) {
        local_1c = 0;
        do {
          if ((local_c == 0) && (iVar7 = 0, 0 < param_4)) {
            do {
              iVar12 = param_2[5];
              *(int *)(puVar10 + -4) = param_1 + 4;
              *(int *)(puVar10 + -8) = iVar12;
              *(undefined4 *)(puVar10 + -0xc) = 0x4a92a0;
              uVar8 = Huffman_DecodeAndRemap(*(int *)(puVar10 + -8),*(uint **)(puVar10 + -4));
              if (uVar8 == 0xffffffff) {
                return 0;
              }
              *(undefined4 *)
               (local_1c + *(int *)(&stack0xffffffc0 + iVar7 * 4 + iVar5 + -0x40 + 0x40)) =
                   *(undefined4 *)(param_2[8] + uVar8 * 4);
              if (*(int *)(local_1c + *(int *)(&stack0xffffffc0 + iVar7 * 4 + iVar5 + -0x40 + 0x40))
                  == 0) {
                return 0;
              }
              iVar7 = iVar7 + 1;
              iVar12 = local_10;
            } while (iVar7 < param_4);
          }
          local_8 = 0;
          if (0 < iVar2) {
            local_18 = iVar12 * iVar3;
            do {
              piVar11 = (int *)(&stack0xffffffc0 + iVar5);
              if (iVar6 <= iVar12) break;
              local_24 = 0;
              if (0 < param_4) {
                iVar7 = param_3 - (int)piVar11;
                do {
                  iVar12 = *piVar1;
                  iVar4 = *(int *)(*(int *)(local_1c + *piVar11) + local_8 * 4);
                  if (((piVar1[iVar4 + 5] & 1 << ((byte)local_c & 0x1f)) != 0) &&
                     (iVar4 = *(int *)(*(int *)(param_2[6] + iVar4 * 4) + local_c * 4), iVar4 != 0))
                  {
                    *(undefined4 *)(puVar10 + -4) = 0xfffffff8;
                    *(int *)(puVar10 + -8) = iVar3;
                    *(int *)(puVar10 + -0xc) = param_1 + 4;
                    *(int *)(puVar10 + -0x10) =
                         *(int *)(iVar7 + (int)piVar11) + (iVar12 + local_18) * 4;
                    *(int *)(puVar10 + -0x14) = iVar4;
                    puVar9 = puVar10 + -0x18;
                    *(undefined4 *)(puVar10 + -0x18) = 0x4a9369;
                    iVar12 = (*(code *)param_5)();
                    puVar10 = puVar9 + 0x14;
                    if (iVar12 == -1) {
                      return 0;
                    }
                  }
                  local_24 = local_24 + 1;
                  piVar11 = piVar11 + 1;
                  iVar12 = local_10;
                } while (local_24 < param_4);
              }
              local_8 = local_8 + 1;
              local_18 = local_18 + iVar3;
              iVar12 = iVar12 + 1;
              local_10 = iVar12;
            } while (local_8 < iVar2);
          }
          local_1c = local_1c + 4;
        } while (iVar12 < iVar6);
      }
      local_c = local_c + 1;
    } while (local_c < param_2[3]);
  }
  return 0;
}

