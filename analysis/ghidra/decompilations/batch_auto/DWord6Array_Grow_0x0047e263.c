
undefined4 DWord6Array_Grow(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  uint uVar3;
  HANDLE hHeap;
  void *pvVar4;
  undefined4 *puVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  int unaff_EBP;
  DWORD dwFlags;
  LPCVOID lpMem;
  
  __security_init_cookie();
  iVar8 = **(int **)(unaff_EBP + 8);
  *(int *)(unaff_EBP + -0x10) = iVar8;
  if (iVar8 == 0) {
    lpMem = (LPCVOID)0x0;
    dwFlags = 0;
    hHeap = GetProcessHeap();
    HeapValidate(hHeap,dwFlags,lpMem);
    iVar8 = *(int *)(unaff_EBP + 0xc);
    pvVar4 = operator_new(iVar8 * 6);
    *(void **)(unaff_EBP + -0x14) = pvVar4;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      RepeatCall(pvVar4,6,iVar8,&LAB_0047d949);
    }
    *(void **)(unaff_EBP + -0x10) = pvVar4;
    **(int **)(unaff_EBP + 0x14) = iVar8;
  }
  else {
    uVar3 = *(uint *)(unaff_EBP + 0xc);
    if (**(uint **)(unaff_EBP + 0x14) < uVar3) {
      pvVar4 = operator_new(uVar3 * 6);
      *(void **)(unaff_EBP + -0x14) = pvVar4;
      *(undefined4 *)(unaff_EBP + -4) = 1;
      if (pvVar4 == (void *)0x0) {
        *(undefined4 *)(unaff_EBP + -0x14) = 0;
      }
      else {
        RepeatCall(pvVar4,6,uVar3,&LAB_0047d949);
        *(void **)(unaff_EBP + -0x14) = pvVar4;
      }
      puVar5 = *(undefined4 **)(unaff_EBP + -0x14);
      *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
      if (puVar5 == (undefined4 *)0x0) {
        uVar6 = 0;
        goto LAB_0047e34a;
      }
      iVar8 = *(int *)(unaff_EBP + 0x10);
      if (iVar8 != 0) {
        iVar7 = *(int *)(unaff_EBP + -0x10) - (int)puVar5;
        do {
          puVar1 = (undefined4 *)(iVar7 + (int)puVar5);
          puVar2 = puVar5 + 1;
          *puVar5 = *puVar1;
          puVar5 = (undefined4 *)((int)puVar5 + 6);
          iVar8 = iVar8 + -1;
          *(undefined2 *)puVar2 = *(undefined2 *)(puVar1 + 1);
        } while (iVar8 != 0);
      }
      pvVar4 = *(void **)(unaff_EBP + -0x10);
      **(uint **)(unaff_EBP + 0x14) = uVar3;
      _free(pvVar4);
      *(undefined4 *)(unaff_EBP + -0x10) = *(undefined4 *)(unaff_EBP + -0x14);
    }
  }
  **(undefined4 **)(unaff_EBP + 8) = *(undefined4 *)(unaff_EBP + -0x10);
  uVar6 = 1;
LAB_0047e34a:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar6;
}

