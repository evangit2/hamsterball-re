
undefined4 D3DX_AllocVertexProcessBuffer(void)

{
  int iVar1;
  void *pvVar2;
  void *pvVar3;
  undefined4 uVar4;
  int extraout_ECX;
  int unaff_EBP;
  
  __security_init_cookie();
  if (*(int *)(extraout_ECX + 8) != *(int *)(*(int *)(unaff_EBP + 8) + 8)) {
    iVar1 = *(int *)(extraout_ECX + 0x1058);
    pvVar2 = operator_new(iVar1 << 4);
    *(void **)(unaff_EBP + -0x10) = pvVar2;
    *(undefined4 *)(unaff_EBP + -4) = 0;
    pvVar3 = (void *)0x0;
    if (pvVar2 != (void *)0x0) {
      RepeatCall(pvVar2,0x10,iVar1,&LAB_0047d949);
      pvVar3 = pvVar2;
    }
    *(void **)(extraout_ECX + 0x104c) = pvVar3;
    if (pvVar3 == (void *)0x0) {
      uVar4 = 0x8007000e;
      goto LAB_00493662;
    }
    *(undefined4 *)(extraout_ECX + 0x1048) = *(undefined4 *)(*(int *)(unaff_EBP + 8) + 8);
  }
  uVar4 = 0;
LAB_00493662:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return uVar4;
}

