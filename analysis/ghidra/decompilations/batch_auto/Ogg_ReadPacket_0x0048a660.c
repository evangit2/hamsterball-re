
undefined4 __cdecl Ogg_ReadPacket(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 *unaff_EBX;
  int unaff_ESI;
  undefined4 *puVar3;
  undefined4 local_14 [5];
  
  if (unaff_EBX != (undefined4 *)0x0) {
    Pool_FreeList((int *)*unaff_EBX);
    puVar3 = unaff_EBX;
    for (iVar2 = 8; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar3 = 0;
      puVar3 = puVar3 + 1;
    }
  }
  Ogg_DecodePacket();
  iVar2 = *(int *)(unaff_ESI + 0x38);
  if ((iVar2 != 0) &&
     (*(uint *)(unaff_ESI + 0x38) = (uint)(*(int *)(unaff_ESI + 0x40) == 0), iVar2 == 2)) {
    *(uint *)(unaff_ESI + 0x24) =
         *(int *)(unaff_ESI + 0x24) + (uint)(0xfffffffe < *(uint *)(unaff_ESI + 0x20));
    *(uint *)(unaff_ESI + 0x20) = *(uint *)(unaff_ESI + 0x20) + 1;
    return 0xfffffff6;
  }
  iVar2 = *(int *)(unaff_ESI + 0x3c);
  if ((iVar2 != 0) &&
     (*(uint *)(unaff_ESI + 0x3c) = (uint)(*(int *)(unaff_ESI + 0x40) == 0), iVar2 == 2)) {
    *(uint *)(unaff_ESI + 0x24) =
         *(int *)(unaff_ESI + 0x24) + (uint)(0xfffffffe < *(uint *)(unaff_ESI + 0x20));
    *(uint *)(unaff_ESI + 0x20) = *(uint *)(unaff_ESI + 0x20) + 1;
    return 0xfffffff5;
  }
  if ((*(uint *)(unaff_ESI + 0x34) & 0x80000000) == 0) {
    return 0;
  }
  if (unaff_EBX == (undefined4 *)0x0) {
    if (param_1 == 0) {
      return 1;
    }
  }
  else {
    unaff_EBX[2] = *(undefined4 *)(unaff_ESI + 0x14);
    if ((*(int *)(unaff_ESI + 0x10) == 0) || (*(int *)(unaff_ESI + 0x48) != 0)) {
      unaff_EBX[3] = 0;
    }
    else {
      unaff_EBX[3] = *(int *)(unaff_ESI + 0x10);
    }
    if (((*(uint *)(unaff_ESI + 0x34) & 0x80000000) == 0) ||
       ((*(uint *)(unaff_ESI + 0x48) & 0x80000000) != 0)) {
      unaff_EBX[4] = 0xffffffff;
      unaff_EBX[5] = 0xffffffff;
    }
    else {
      unaff_EBX[4] = *(undefined4 *)(unaff_ESI + 0x28);
      unaff_EBX[5] = *(undefined4 *)(unaff_ESI + 0x2c);
    }
    unaff_EBX[6] = *(undefined4 *)(unaff_ESI + 0x20);
    unaff_EBX[7] = *(undefined4 *)(unaff_ESI + 0x24);
    if (param_1 == 0) {
      piVar1 = FragmentList_SplitRange(0,*(int **)(unaff_ESI + 0xc));
      *unaff_EBX = piVar1;
      unaff_EBX[1] = *(uint *)(unaff_ESI + 0x34) & 0x7fffffff;
      return 1;
    }
  }
  StreamReader_InitFromBuffer(local_14);
  if (unaff_EBX == (undefined4 *)0x0) {
    iVar2 = D3DX_SurfaceClipBlit(*(uint *)(unaff_ESI + 0x34) & 0x7fffffff);
    *(int *)(unaff_ESI + 0xc) = iVar2;
    if (iVar2 == 0) {
      *(undefined4 *)(unaff_ESI + 8) = 0;
    }
  }
  else {
    piVar1 = AthenaList_SplitChunk((int *)(unaff_ESI + 8),*(uint *)(unaff_ESI + 0x34) & 0x7fffffff);
    *unaff_EBX = piVar1;
    unaff_EBX[1] = *(uint *)(unaff_ESI + 0x34) & 0x7fffffff;
  }
  *(undefined4 *)(unaff_ESI + 0x34) = *(undefined4 *)(unaff_ESI + 0x48);
  Ogg_CalculateCRC();
  *(uint *)(unaff_ESI + 0x24) =
       *(int *)(unaff_ESI + 0x24) + (uint)(0xfffffffe < *(uint *)(unaff_ESI + 0x20));
  *(undefined4 *)(unaff_ESI + 0x14) = 0;
  *(uint *)(unaff_ESI + 0x20) = *(uint *)(unaff_ESI + 0x20) + 1;
  return 1;
}

