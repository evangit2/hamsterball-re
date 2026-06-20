
undefined4 __cdecl Vorbis_InitDecodeChannel(int param_1)

{
  int *piVar1;
  undefined4 *puVar2;
  uint uVar3;
  undefined *puVar4;
  void *pvVar5;
  undefined4 uVar6;
  int iVar7;
  undefined4 *unaff_EBX;
  int *piVar8;
  int local_8;
  
  piVar1 = *(int **)(param_1 + 0x1c);
  puVar2 = unaff_EBX;
  for (iVar7 = 0x14; iVar7 != 0; iVar7 = iVar7 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  puVar2 = _calloc(1,0x18);
  unaff_EBX[0x12] = puVar2;
  unaff_EBX[1] = param_1;
  iVar7 = 0;
  if (piVar1[2] != 0) {
    for (uVar3 = piVar1[2] - 1; uVar3 != 0; uVar3 = uVar3 >> 1) {
      iVar7 = iVar7 + 1;
    }
  }
  puVar2[2] = iVar7;
  puVar4 = TrigTable_Lookup(0,*piVar1 / 2);
  *puVar2 = puVar4;
  puVar4 = TrigTable_Lookup(0,piVar1[1] / 2);
  puVar2[1] = puVar4;
  if (piVar1[0x308] == 0) {
    pvVar5 = _calloc(piVar1[7],0x34);
    piVar1[0x308] = (int)pvVar5;
    local_8 = 0;
    if (0 < piVar1[7]) {
      iVar7 = 0;
      piVar8 = piVar1 + 0x208;
      do {
        Huffman_BuildDecodeTable((undefined4 *)(piVar1[0x308] + iVar7),(undefined4 *)*piVar8);
        Huffman_FreeTable((undefined4 *)*piVar8);
        *piVar8 = 0;
        local_8 = local_8 + 1;
        piVar8 = piVar8 + 1;
        iVar7 = iVar7 + 0x34;
      } while (local_8 < piVar1[7]);
    }
  }
  unaff_EBX[4] = piVar1[1];
  pvVar5 = _malloc(*(int *)(param_1 + 4) << 2);
  unaff_EBX[2] = pvVar5;
  pvVar5 = _malloc(*(int *)(param_1 + 4) << 2);
  unaff_EBX[3] = pvVar5;
  iVar7 = 0;
  if (0 < *(int *)(param_1 + 4)) {
    do {
      pvVar5 = _calloc(unaff_EBX[4],4);
      *(void **)(unaff_EBX[2] + iVar7 * 4) = pvVar5;
      iVar7 = iVar7 + 1;
    } while (iVar7 < *(int *)(param_1 + 4));
  }
  unaff_EBX[9] = 0;
  unaff_EBX[10] = 0;
  pvVar5 = _calloc(piVar1[2],4);
  puVar2[3] = pvVar5;
  iVar7 = 0;
  if (0 < piVar1[2]) {
    piVar8 = piVar1 + 8;
    do {
      uVar6 = (**(code **)((&PTR_PTR_004fa7d8)[piVar1[*(int *)(*piVar8 + 0xc) + 0x48]] + 4))();
      *(undefined4 *)((int)piVar8 + puVar2[3] + (-0x20 - (int)piVar1)) = uVar6;
      iVar7 = iVar7 + 1;
      piVar8 = piVar8 + 1;
    } while (iVar7 < piVar1[2]);
  }
  return 0;
}

