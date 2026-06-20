
void __thiscall PNG_ReadIDATData(void *this,int *param_1)

{
  uint *puVar1;
  byte bVar2;
  int *piVar3;
  uint uVar4;
  byte *pbVar5;
  char *pcVar6;
  int iVar7;
  void *local_4;
  
  piVar3 = param_1;
  puVar1 = (uint *)(param_1 + 0x35);
  *puVar1 = *puVar1 + 1;
  if ((uint)param_1[0x30] <= *puVar1) {
    local_4 = this;
    if (*(char *)((int)param_1 + 0x113) != '\0') {
      *puVar1 = 0;
      Zlib_MemSet(param_1,(undefined4 *)param_1[0x36],0,param_1[0x32] + 1);
      do {
        *(char *)(piVar3 + 0x45) = (char)piVar3[0x45] + '\x01';
        bVar2 = *(byte *)(piVar3 + 0x45);
        if (6 < bVar2) goto LAB_004ae7ca;
        iVar7 = (uint)bVar2 * 4;
        uVar4 = ((piVar3[0x2e] - *(int *)(&DAT_004e67b0 + iVar7)) + -1 +
                *(uint *)(&DAT_004e67cc + iVar7)) / *(uint *)(&DAT_004e67cc + iVar7);
        piVar3[0x34] = uVar4;
        piVar3[0x33] = (*(byte *)((int)piVar3 + 0x119) * uVar4 + 7 >> 3) + 1;
      } while (((*(byte *)(piVar3 + 0x18) & 2) == 0) &&
              (piVar3[0x30] = ((piVar3[0x2f] - *(int *)(&DAT_004e67e8 + iVar7)) + -1 +
                              *(uint *)(&DAT_004e6804 + iVar7)) / *(uint *)(&DAT_004e6804 + iVar7),
              piVar3[0x34] == 0));
      if (bVar2 < 7) {
        return;
      }
    }
LAB_004ae7ca:
    if ((*(byte *)(piVar3 + 0x17) & 0x20) == 0) {
      piVar3[0x1c] = (int)&param_1;
      piVar3[0x1d] = 1;
      while( true ) {
        if (piVar3[0x1a] == 0) {
          if (piVar3[0x3f] == 0) {
            do {
              Zlib_FlushWithCRC(piVar3,0);
              FileStream_ReadCallback(piVar3,&local_4,4);
              iVar7 = ReadInt32BE((undefined1 *)&local_4);
              piVar3[0x3f] = iVar7;
              PNG_InitCRC32((int)piVar3);
              Zlib_UpdateHash(piVar3,(byte *)(piVar3 + 0x43),4);
              if (piVar3[0x43] != DAT_004e6820) {
                longjmp_with_cleanup(piVar3,"Not enough image data");
              }
            } while (piVar3[0x3f] == 0);
          }
          piVar3[0x1a] = piVar3[0x28];
          piVar3[0x19] = piVar3[0x27];
          if ((uint)piVar3[0x3f] < (uint)piVar3[0x28]) {
            piVar3[0x1a] = piVar3[0x3f];
          }
          Zlib_UpdateHash(piVar3,(byte *)piVar3[0x27],piVar3[0x1a]);
          piVar3[0x3f] = piVar3[0x3f] - piVar3[0x1a];
        }
        pbVar5 = Zlib_Inflate((byte *)(piVar3 + 0x19),1);
        if (pbVar5 == (byte *)0x1) break;
        if (pbVar5 != (byte *)0x0) {
          pcVar6 = (char *)piVar3[0x1f];
          if (pcVar6 == (char *)0x0) {
            pcVar6 = "Decompression Error";
          }
          longjmp_with_cleanup(piVar3,pcVar6);
        }
        if (piVar3[0x1d] == 0) {
          longjmp_with_cleanup(piVar3,"Extra compressed data");
        }
      }
      if (((piVar3[0x1d] == 0) || (piVar3[0x1a] != 0)) || (piVar3[0x3f] != 0)) {
        longjmp_with_cleanup(piVar3,"Extra compressed data");
      }
      piVar3[0x16] = piVar3[0x16] | 8;
      piVar3[0x17] = piVar3[0x17] | 0x20;
      piVar3[0x1d] = 0;
    }
    if ((piVar3[0x3f] != 0) || (piVar3[0x1a] != 0)) {
      longjmp_with_cleanup(piVar3,"Extra compression data");
    }
    Audio_CodecResetAndDecode((int)(piVar3 + 0x19));
    piVar3[0x16] = piVar3[0x16] | 8;
  }
  return;
}

