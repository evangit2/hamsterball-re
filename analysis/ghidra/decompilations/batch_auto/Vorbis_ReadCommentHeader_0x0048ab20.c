
undefined4 __cdecl Vorbis_ReadCommentHeader(uint *param_1)

{
  uint uVar1;
  undefined1 *puVar2;
  uint uVar3;
  void *pvVar4;
  int iVar5;
  int *unaff_EDI;
  
  uVar1 = BitStream_ReadBits(param_1,0x20);
  if (-1 < (int)uVar1) {
    puVar2 = _calloc(uVar1 + 1,1);
    unaff_EDI[3] = (int)puVar2;
    for (; uVar1 != 0; uVar1 = uVar1 - 1) {
      uVar3 = BitStream_ReadBits(param_1,8);
      *puVar2 = (char)uVar3;
      puVar2 = puVar2 + 1;
    }
    uVar1 = BitStream_ReadBits(param_1,0x20);
    unaff_EDI[2] = uVar1;
    if (-1 < (int)uVar1) {
      pvVar4 = _calloc(uVar1 + 1,4);
      *unaff_EDI = (int)pvVar4;
      pvVar4 = _calloc(unaff_EDI[2] + 1,4);
      unaff_EDI[1] = (int)pvVar4;
      iVar5 = 0;
      if (0 < unaff_EDI[2]) {
        do {
          uVar1 = BitStream_ReadBits(param_1,0x20);
          if ((int)uVar1 < 0) goto LAB_0048ac05;
          *(uint *)(unaff_EDI[1] + iVar5 * 4) = uVar1;
          pvVar4 = _calloc(uVar1 + 1,1);
          *(void **)(*unaff_EDI + iVar5 * 4) = pvVar4;
          puVar2 = *(undefined1 **)(*unaff_EDI + iVar5 * 4);
          for (; uVar1 != 0; uVar1 = uVar1 - 1) {
            uVar3 = BitStream_ReadBits(param_1,8);
            *puVar2 = (char)uVar3;
            puVar2 = puVar2 + 1;
          }
          iVar5 = iVar5 + 1;
        } while (iVar5 < unaff_EDI[2]);
      }
      uVar1 = BitStream_ReadBits(param_1,1);
      if (uVar1 == 1) {
        return 0;
      }
    }
  }
LAB_0048ac05:
  MeshGroup_dtor(unaff_EDI);
  return 0xffffff7b;
}

