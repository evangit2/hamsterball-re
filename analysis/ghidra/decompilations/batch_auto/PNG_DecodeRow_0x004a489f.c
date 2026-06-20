
void __thiscall PNG_DecodeRow(void *this,int *param_1,byte *param_2,byte *param_3)

{
  uint *puVar1;
  char cVar2;
  byte bVar3;
  int *piVar4;
  int iVar5;
  byte *pbVar6;
  char *pcVar7;
  int *extraout_ECX;
  int *extraout_ECX_00;
  int *this_00;
  int *extraout_ECX_01;
  int *extraout_ECX_02;
  int *extraout_ECX_03;
  int *extraout_ECX_04;
  bool bVar8;
  uint uVar9;
  
  piVar4 = param_1;
  if ((*(byte *)(param_1 + 0x17) & 0x40) == 0) {
    this = param_1;
    PNG_SetupRowTransform(param_1);
  }
  if ((*(char *)((int)piVar4 + 0x113) != '\0') && ((*(byte *)(piVar4 + 0x18) & 2) != 0)) {
    cVar2 = (char)piVar4[0x45];
    if (cVar2 == '\0') {
      if ((*(byte *)(piVar4 + 0x35) & 7) != 0) {
        bVar8 = param_3 == (byte *)0x0;
LAB_004a49b1:
        if (bVar8) goto LAB_004a4907;
        uVar9 = 0xff;
LAB_004a49bc:
        Font_DecodeGlyphBits((int)piVar4,param_3,uVar9);
        this = extraout_ECX;
LAB_004a4907:
        PNG_ReadIDATData(this,piVar4);
        return;
      }
    }
    else if (cVar2 == '\x01') {
      if (((*(byte *)(piVar4 + 0x35) & 7) != 0) || ((uint)piVar4[0x2e] < 5)) {
        if (param_3 == (byte *)0x0) goto LAB_004a4907;
        uVar9 = 0xf;
        goto LAB_004a49bc;
      }
    }
    else if (cVar2 == '\x02') {
      this = (void *)(piVar4[0x35] & 0xffffff07);
      if ((char)this != '\x04') {
        if (param_3 == (byte *)0x0) goto LAB_004a4907;
        bVar8 = (piVar4[0x35] & 4U) == 0;
        goto LAB_004a49b1;
      }
    }
    else if (cVar2 == '\x03') {
      if (((*(byte *)(piVar4 + 0x35) & 3) != 0) || ((uint)piVar4[0x2e] < 3)) {
        if (param_3 == (byte *)0x0) goto LAB_004a4907;
        uVar9 = 0x33;
        goto LAB_004a49bc;
      }
    }
    else if (cVar2 == '\x04') {
      this = (void *)(piVar4[0x35] & 0xffffff03);
      if ((char)this != '\x02') {
        if (param_3 == (byte *)0x0) goto LAB_004a4907;
        bVar8 = (piVar4[0x35] & 2U) == 0;
        goto LAB_004a49b1;
      }
    }
    else if (cVar2 == '\x05') {
      if (((*(byte *)(piVar4 + 0x35) & 1) != 0) || ((uint)piVar4[0x2e] < 2)) {
        if (param_3 == (byte *)0x0) goto LAB_004a4907;
        uVar9 = 0x55;
        goto LAB_004a49bc;
      }
    }
    else if ((cVar2 == '\x06') && ((*(byte *)(piVar4 + 0x35) & 1) == 0)) goto LAB_004a4907;
  }
  if ((*(byte *)(piVar4 + 0x16) & 4) == 0) {
    longjmp_with_cleanup(piVar4,"Invalid attempt to read row data");
  }
  piVar4[0x1c] = piVar4[0x37];
  piVar4[0x1d] = piVar4[0x33];
  do {
    if (piVar4[0x1a] == 0) {
      if (piVar4[0x3f] == 0) {
        do {
          Zlib_FlushWithCRC(piVar4,0);
          FileStream_ReadCallback(piVar4,&param_1,4);
          iVar5 = ReadInt32BE((undefined1 *)&param_1);
          piVar4[0x3f] = iVar5;
          PNG_InitCRC32((int)piVar4);
          Zlib_UpdateHash(piVar4,(byte *)(piVar4 + 0x43),4);
          if (piVar4[0x43] != DAT_004e56ac) {
            longjmp_with_cleanup(piVar4,"Not enough image data");
          }
        } while (piVar4[0x3f] == 0);
      }
      piVar4[0x1a] = piVar4[0x28];
      piVar4[0x19] = piVar4[0x27];
      if ((uint)piVar4[0x3f] < (uint)piVar4[0x28]) {
        piVar4[0x1a] = piVar4[0x3f];
      }
      Zlib_UpdateHash(piVar4,(byte *)piVar4[0x27],piVar4[0x1a]);
      piVar4[0x3f] = piVar4[0x3f] - piVar4[0x1a];
    }
    pbVar6 = Zlib_Inflate((byte *)(piVar4 + 0x19),1);
    if (pbVar6 == (byte *)0x1) {
      if (((piVar4[0x1d] != 0) || (piVar4[0x1a] != 0)) || (piVar4[0x3f] != 0)) {
        longjmp_with_cleanup(piVar4,"Extra compressed data");
      }
      piVar4[0x16] = piVar4[0x16] | 8;
      piVar4[0x17] = piVar4[0x17] | 0x20;
      break;
    }
    if (pbVar6 != (byte *)0x0) {
      pcVar7 = (char *)piVar4[0x1f];
      if (pcVar7 == (char *)0x0) {
        pcVar7 = "Decompression error";
      }
      longjmp_with_cleanup(piVar4,pcVar7);
    }
  } while (piVar4[0x1d] != 0);
  *(undefined1 *)((int)piVar4 + 0xfa) = *(undefined1 *)((int)piVar4 + 0x11a);
  *(undefined1 *)((int)piVar4 + 0xf9) = *(undefined1 *)((int)piVar4 + 0x117);
  *(byte *)((int)piVar4 + 0xfb) = *(byte *)((int)piVar4 + 0x119);
  *(undefined1 *)(piVar4 + 0x3e) = *(undefined1 *)((int)piVar4 + 0x116);
  puVar1 = (uint *)(piVar4 + 0x3c);
  *puVar1 = piVar4[0x34];
  piVar4[0x3d] = (uint)*(byte *)((int)piVar4 + 0x119) * piVar4[0x34] + 7 >> 3;
  PNG_ReconstructFilter
            ((int)piVar4,(int)puVar1,(byte *)piVar4[0x37] + 1,(byte *)(piVar4[0x36] + 1),
             (uint)*(byte *)piVar4[0x37]);
  Zlib_MemCopy(piVar4,(undefined4 *)piVar4[0x36],(undefined4 *)piVar4[0x37],piVar4[0x32] + 1);
  this_00 = extraout_ECX_00;
  if (piVar4[0x18] != 0) {
    this_00 = piVar4;
    PNG_ProcessRowTransforms(piVar4);
  }
  if ((*(char *)((int)piVar4 + 0x113) == '\0') || ((piVar4[0x18] & 2U) == 0)) {
    if (param_2 != (byte *)0x0) {
      Font_DecodeGlyphBits((int)piVar4,param_2,0xff);
      this_00 = extraout_ECX_03;
    }
    if (param_3 == (byte *)0x0) goto LAB_004a4c1c;
    uVar9 = 0xff;
    pbVar6 = param_3;
  }
  else {
    bVar3 = *(byte *)(piVar4 + 0x45);
    this_00 = (int *)CONCAT31((int3)((uint)this_00 >> 8),bVar3);
    if (bVar3 < 6) {
      PNG_DeinterlaceRow(puVar1,(int *)(piVar4[0x37] + 1),(uint)bVar3);
      this_00 = extraout_ECX_01;
    }
    if (param_3 != (byte *)0x0) {
      Font_DecodeGlyphBits
                ((int)piVar4,param_3,*(uint *)(&DAT_004e5688 + (uint)*(byte *)(piVar4 + 0x45) * 4));
      this_00 = extraout_ECX_02;
    }
    if (param_2 == (byte *)0x0) goto LAB_004a4c1c;
    uVar9 = *(uint *)(&DAT_004e566c + (uint)*(byte *)(piVar4 + 0x45) * 4);
    pbVar6 = param_2;
  }
  Font_DecodeGlyphBits((int)piVar4,pbVar6,uVar9);
  this_00 = extraout_ECX_04;
LAB_004a4c1c:
  PNG_ReadIDATData(this_00,piVar4);
  if ((code *)piVar4[0x5b] != (code *)0x0) {
    (*(code *)piVar4[0x5b])(piVar4,piVar4[0x35],(char)piVar4[0x45]);
  }
  return;
}

