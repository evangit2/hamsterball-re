
undefined4 D3DX_CopyRects_Point(void)

{
  int iVar1;
  undefined4 uVar2;
  void *pvVar3;
  int *extraout_ECX;
  int iVar4;
  int *piVar5;
  int unaff_EBP;
  uint uVar6;
  undefined4 *puVar7;
  uint uVar8;
  uint uVar9;
  
  __security_init_cookie();
  if ((char)extraout_ECX[2] == '\x01') {
    iVar1 = extraout_ECX[1];
    iVar4 = *extraout_ECX;
    uVar6 = *(uint *)(iVar1 + 0x1058);
    uVar8 = *(uint *)(iVar4 + 0x1058);
    *(uint *)(unaff_EBP + -0x18) = uVar6;
    if (uVar6 <= uVar8) {
      *(uint *)(unaff_EBP + -0x18) = uVar8;
    }
    uVar6 = *(uint *)(iVar1 + 0x105c);
    uVar8 = *(uint *)(iVar4 + 0x105c);
    *(uint *)(unaff_EBP + -0x20) = uVar6;
    if (uVar8 <= uVar6) {
      *(uint *)(unaff_EBP + -0x20) = uVar8;
    }
    uVar6 = *(uint *)(iVar1 + 0x1060);
    uVar8 = *(uint *)(iVar4 + 0x1060);
    *(uint *)(unaff_EBP + -0x1c) = uVar6;
    if (uVar8 <= uVar6) {
      *(uint *)(unaff_EBP + -0x1c) = uVar8;
    }
    uVar6 = *(int *)(unaff_EBP + -0x18) << 4;
    pvVar3 = operator_new(uVar6);
    *(void **)(unaff_EBP + -0x14) = pvVar3;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    if (pvVar3 == (void *)0x0) {
      *(undefined4 *)(unaff_EBP + -0x10) = 0;
    }
    else {
      RepeatCall(pvVar3,0x10,*(int *)(unaff_EBP + -0x18),&LAB_0047d949);
      *(undefined4 *)(unaff_EBP + -0x10) = *(undefined4 *)(unaff_EBP + -0x14);
    }
    *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
    if (*(int *)(unaff_EBP + -0x10) == 0) {
      uVar2 = 0x80004005;
    }
    else {
      iVar1 = *(int *)(extraout_ECX[1] + 0x1058);
      *(int *)(unaff_EBP + -0x18) = iVar1;
      pvVar3 = operator_new(iVar1 << 4);
      *(void **)(unaff_EBP + -0x14) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 1;
      if (pvVar3 == (void *)0x0) {
        *(undefined4 *)(unaff_EBP + -0x18) = 0;
      }
      else {
        RepeatCall(pvVar3,0x10,*(int *)(unaff_EBP + -0x18),&LAB_0047d949);
        *(undefined4 *)(unaff_EBP + -0x18) = *(undefined4 *)(unaff_EBP + -0x14);
      }
      *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
      if (*(int *)(unaff_EBP + -0x18) == 0) {
        _free(*(void **)(unaff_EBP + -0x10));
        uVar2 = 0x80004005;
      }
      else {
        iVar1 = *(int *)(unaff_EBP + -0x1c);
        *(undefined4 *)(unaff_EBP + -0x14) = 0;
        puVar7 = *(undefined4 **)(unaff_EBP + -0x10);
        for (uVar6 = uVar6 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
          *puVar7 = 0;
          puVar7 = puVar7 + 1;
        }
        for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
          *(undefined1 *)puVar7 = 0;
          puVar7 = (undefined4 *)((int)puVar7 + 1);
        }
        puVar7 = *(undefined4 **)(unaff_EBP + -0x18);
        for (uVar6 = (uint)(*(int *)(extraout_ECX[1] + 0x1058) << 4) >> 2; uVar6 != 0;
            uVar6 = uVar6 - 1) {
          *puVar7 = 0;
          puVar7 = puVar7 + 1;
        }
        for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
          *(undefined1 *)puVar7 = 0;
          puVar7 = (undefined4 *)((int)puVar7 + 1);
        }
        uVar6 = 0;
        if (iVar1 != 0) {
          do {
            uVar6 = *(uint *)(unaff_EBP + -0x20);
            uVar8 = 0;
            if (uVar6 != 0) {
              do {
                (**(code **)(*(int *)*extraout_ECX + 4))
                          (uVar8,*(undefined4 *)(unaff_EBP + -0x14),
                           *(undefined4 *)(unaff_EBP + -0x10));
                (**(code **)(*(int *)extraout_ECX[1] + 8))
                          (uVar8,*(undefined4 *)(unaff_EBP + -0x14),
                           *(undefined4 *)(unaff_EBP + -0x10));
                uVar8 = uVar8 + 1;
              } while (uVar8 < uVar6);
            }
            piVar5 = (int *)extraout_ECX[1];
            if (uVar6 < (uint)piVar5[0x417]) {
              do {
                (**(code **)(*piVar5 + 8))
                          (uVar6,*(undefined4 *)(unaff_EBP + -0x14),
                           *(undefined4 *)(unaff_EBP + -0x18));
                piVar5 = (int *)extraout_ECX[1];
                uVar6 = uVar6 + 1;
              } while (uVar6 < (uint)piVar5[0x417]);
            }
            uVar6 = *(uint *)(unaff_EBP + -0x1c);
            *(int *)(unaff_EBP + -0x14) = *(int *)(unaff_EBP + -0x14) + 1;
          } while (*(uint *)(unaff_EBP + -0x14) < uVar6);
        }
        piVar5 = (int *)extraout_ECX[1];
        if (uVar6 < (uint)piVar5[0x418]) {
          uVar8 = piVar5[0x417];
          do {
            uVar9 = 0;
            if (uVar8 != 0) {
              do {
                (**(code **)(*piVar5 + 8))(uVar9,uVar6,*(undefined4 *)(unaff_EBP + -0x18));
                piVar5 = (int *)extraout_ECX[1];
                uVar8 = piVar5[0x417];
                uVar9 = uVar9 + 1;
              } while (uVar9 < uVar8);
            }
            piVar5 = (int *)extraout_ECX[1];
            uVar6 = uVar6 + 1;
          } while (uVar6 < (uint)piVar5[0x418]);
        }
        _free(*(void **)(unaff_EBP + -0x10));
        _free(*(void **)(unaff_EBP + -0x18));
        uVar2 = 0;
      }
    }
  }
  else {
    uVar2 = 0x80004005;
  }
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar2;
}

