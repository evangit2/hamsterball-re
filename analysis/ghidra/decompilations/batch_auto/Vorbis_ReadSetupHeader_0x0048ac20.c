
undefined4 __cdecl Vorbis_ReadSetupHeader(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  uint *puVar3;
  int iVar4;
  void *pvVar5;
  int iVar6;
  uint *unaff_ESI;
  undefined4 *puVar7;
  int *piVar8;
  
  iVar1 = param_1[7];
  if (iVar1 == 0) {
    return 0xffffff7f;
  }
  uVar2 = BitStream_ReadBits(unaff_ESI,8);
  iVar6 = 0;
  *(uint *)(iVar1 + 0x1c) = uVar2 + 1;
  if (0 < (int)(uVar2 + 1)) {
    puVar7 = (undefined4 *)(iVar1 + 0x820);
    do {
      puVar3 = _calloc(1,0x24);
      *puVar7 = puVar3;
      iVar4 = Huffman_ReadVCBHeader(unaff_ESI,puVar3);
      if (iVar4 != 0) goto LAB_0048ae82;
      iVar6 = iVar6 + 1;
      puVar7 = puVar7 + 1;
    } while (iVar6 < *(int *)(iVar1 + 0x1c));
  }
  uVar2 = BitStream_ReadBits(unaff_ESI,6);
  iVar6 = 0;
  *(uint *)(iVar1 + 0x10) = uVar2 + 1;
  if (0 < (int)(uVar2 + 1)) {
    puVar3 = (uint *)(iVar1 + 800);
    do {
      uVar2 = BitStream_ReadBits(unaff_ESI,0x10);
      *puVar3 = uVar2;
      if (((int)uVar2 < 0) || (0 < (int)uVar2)) goto LAB_0048ae82;
      iVar6 = iVar6 + 1;
      puVar3 = puVar3 + 1;
    } while (iVar6 < *(int *)(iVar1 + 0x10));
  }
  uVar2 = BitStream_ReadBits(unaff_ESI,6);
  iVar6 = 0;
  *(uint *)(iVar1 + 0x14) = uVar2 + 1;
  if (0 < (int)(uVar2 + 1)) {
    piVar8 = (int *)(iVar1 + 0x520);
    do {
      uVar2 = BitStream_ReadBits(unaff_ESI,0x10);
      piVar8[-0x40] = uVar2;
      if (((int)uVar2 < 0) || (1 < (int)uVar2)) goto LAB_0048ae82;
      iVar4 = (**(code **)(&PTR_PTR_004fa7c4)[uVar2])(param_1);
      *piVar8 = iVar4;
      if (iVar4 == 0) goto LAB_0048ae82;
      iVar6 = iVar6 + 1;
      piVar8 = piVar8 + 1;
    } while (iVar6 < *(int *)(iVar1 + 0x14));
  }
  uVar2 = BitStream_ReadBits(unaff_ESI,6);
  iVar6 = 0;
  *(uint *)(iVar1 + 0x18) = uVar2 + 1;
  if (0 < (int)(uVar2 + 1)) {
    piVar8 = (int *)(iVar1 + 0x720);
    do {
      uVar2 = BitStream_ReadBits(unaff_ESI,0x10);
      piVar8[-0x40] = uVar2;
      if (((int)uVar2 < 0) || (2 < (int)uVar2)) goto LAB_0048ae82;
      iVar4 = (**(code **)(&PTR_PTR_004fa7cc)[uVar2])(param_1);
      *piVar8 = iVar4;
      if (iVar4 == 0) goto LAB_0048ae82;
      iVar6 = iVar6 + 1;
      piVar8 = piVar8 + 1;
    } while (iVar6 < *(int *)(iVar1 + 0x18));
  }
  uVar2 = BitStream_ReadBits(unaff_ESI,6);
  iVar6 = 0;
  *(uint *)(iVar1 + 0xc) = uVar2 + 1;
  if (0 < (int)(uVar2 + 1)) {
    piVar8 = (int *)(iVar1 + 0x220);
    do {
      uVar2 = BitStream_ReadBits(unaff_ESI,0x10);
      piVar8[-0x40] = uVar2;
      if (((int)uVar2 < 0) || (0 < (int)uVar2)) goto LAB_0048ae82;
      iVar4 = (**(code **)(&PTR_PTR_004fa7d8)[uVar2])(param_1);
      *piVar8 = iVar4;
      if (iVar4 == 0) goto LAB_0048ae82;
      iVar6 = iVar6 + 1;
      piVar8 = piVar8 + 1;
    } while (iVar6 < *(int *)(iVar1 + 0xc));
  }
  uVar2 = BitStream_ReadBits(unaff_ESI,6);
  iVar6 = 0;
  *(uint *)(iVar1 + 8) = uVar2 + 1;
  if (0 < (int)(uVar2 + 1)) {
    piVar8 = (int *)(iVar1 + 0x20);
    do {
      pvVar5 = _calloc(1,0x10);
      *piVar8 = (int)pvVar5;
      uVar2 = BitStream_ReadBits(unaff_ESI,1);
      *(uint *)*piVar8 = uVar2;
      uVar2 = BitStream_ReadBits(unaff_ESI,0x10);
      *(uint *)(*piVar8 + 4) = uVar2;
      uVar2 = BitStream_ReadBits(unaff_ESI,0x10);
      *(uint *)(*piVar8 + 8) = uVar2;
      uVar2 = BitStream_ReadBits(unaff_ESI,8);
      *(uint *)(*piVar8 + 0xc) = uVar2;
      iVar4 = *piVar8;
      if (((0 < *(int *)(iVar4 + 4)) || (0 < *(int *)(iVar4 + 8))) ||
         (*(int *)(iVar1 + 0xc) <= *(int *)(iVar4 + 0xc))) goto LAB_0048ae82;
      iVar6 = iVar6 + 1;
      piVar8 = piVar8 + 1;
    } while (iVar6 < *(int *)(iVar1 + 8));
  }
  uVar2 = BitStream_ReadBits(unaff_ESI,1);
  if (uVar2 == 1) {
    return 0;
  }
LAB_0048ae82:
  D3DResourcePool_Release(param_1);
  return 0xffffff7b;
}

