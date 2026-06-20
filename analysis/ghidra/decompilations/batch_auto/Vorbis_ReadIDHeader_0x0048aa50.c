
undefined4 Vorbis_ReadIDHeader(void)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  uint *unaff_ESI;
  uint *unaff_EDI;
  
  piVar1 = (int *)unaff_EDI[7];
  if (piVar1 == (int *)0x0) {
    return 0xffffff7f;
  }
  uVar2 = BitStream_ReadBits(unaff_ESI,0x20);
  *unaff_EDI = uVar2;
  if (uVar2 != 0) {
    return 0xffffff7a;
  }
  uVar2 = BitStream_ReadBits(unaff_ESI,8);
  unaff_EDI[1] = uVar2;
  uVar2 = BitStream_ReadBits(unaff_ESI,0x20);
  unaff_EDI[2] = uVar2;
  uVar2 = BitStream_ReadBits(unaff_ESI,0x20);
  unaff_EDI[3] = uVar2;
  uVar2 = BitStream_ReadBits(unaff_ESI,0x20);
  unaff_EDI[4] = uVar2;
  uVar2 = BitStream_ReadBits(unaff_ESI,0x20);
  unaff_EDI[5] = uVar2;
  uVar2 = BitStream_ReadBits(unaff_ESI,4);
  *piVar1 = 1 << ((byte)uVar2 & 0x1f);
  uVar2 = BitStream_ReadBits(unaff_ESI,4);
  iVar3 = 1 << ((byte)uVar2 & 0x1f);
  piVar1[1] = iVar3;
  if ((((0 < (int)unaff_EDI[2]) && (0 < (int)unaff_EDI[1])) && (0x3f < *piVar1)) &&
     ((*piVar1 <= iVar3 && (iVar3 < 0x2001)))) {
    uVar2 = BitStream_ReadBits(unaff_ESI,1);
    if (uVar2 == 1) {
      return 0;
    }
  }
  D3DResourcePool_Release(unaff_EDI);
  return 0xffffff7b;
}

