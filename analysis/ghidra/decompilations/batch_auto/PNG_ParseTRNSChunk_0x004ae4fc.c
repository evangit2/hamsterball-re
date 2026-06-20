
void __cdecl PNG_ParseTRNSChunk(int *param_1,int param_2,uint param_3)

{
  char cVar1;
  byte *pbVar2;
  int iVar3;
  char *pcVar4;
  undefined4 local_c;
  ushort local_8;
  
  if ((param_1[0x16] & 1U) == 0) {
    longjmp_with_cleanup(param_1,"Missing IHDR before tRNS");
  }
  else {
    if ((param_1[0x16] & 4U) != 0) {
      pcVar4 = "Invalid tRNS after IDAT";
LAB_004ae571:
      seh_filter_invoke((int)param_1,pcVar4);
      Zlib_FlushWithCRC(param_1,param_3);
      return;
    }
    if ((param_2 != 0) && ((*(byte *)(param_2 + 8) & 0x10) != 0)) {
      pcVar4 = "Duplicate tRNS chunk";
      goto LAB_004ae571;
    }
  }
  cVar1 = *(char *)((int)param_1 + 0x116);
  if (cVar1 == '\x03') {
    if ((*(byte *)(param_1 + 0x16) & 2) == 0) {
      seh_filter_invoke((int)param_1,"Missing PLTE before tRNS");
LAB_004ae542:
      if (param_3 != 0) {
        pbVar2 = Malloc_OrLongjmp(param_1,param_3);
        *(byte *)((int)param_1 + 0x5d) = *(byte *)((int)param_1 + 0x5d) | 0x20;
        param_1[0x57] = (int)pbVar2;
        Zlib_UpdateHash(param_1,pbVar2,param_3);
        *(short *)((int)param_1 + 0x10a) = (short)param_3;
        goto LAB_004ae668;
      }
      seh_filter_invoke((int)param_1,"Zero length tRNS chunk");
    }
    else {
      if (param_3 <= *(ushort *)(param_1 + 0x42)) goto LAB_004ae542;
      seh_filter_invoke((int)param_1,"Incorrect tRNS chunk length");
    }
LAB_004ae6a5:
    Zlib_FlushWithCRC(param_1,param_3);
  }
  else {
    if (cVar1 == '\x02') {
      if (param_3 != 6) {
LAB_004ae633:
        pcVar4 = "Incorrect tRNS chunk length";
LAB_004ae69c:
        seh_filter_invoke((int)param_1,pcVar4);
        goto LAB_004ae6a5;
      }
      Zlib_UpdateHash(param_1,(byte *)&local_c,6);
      *(short *)((int)param_1 + 0x162) = (short)((local_c & 0xff) * 0x100 + (local_c >> 8 & 0xff));
      *(ushort *)(param_1 + 0x59) = (ushort)local_c._2_1_ * 0x100 + (ushort)local_c._3_1_;
      *(ushort *)((int)param_1 + 0x166) = local_8 * 0x100 + (local_8 >> 8);
    }
    else {
      if (cVar1 != '\0') {
        pcVar4 = "tRNS chunk not allowed with alpha channel";
        goto LAB_004ae69c;
      }
      if (param_3 != 2) goto LAB_004ae633;
      Zlib_UpdateHash(param_1,(byte *)&local_c,2);
      *(ushort *)(param_1 + 0x5a) = (ushort)local_c * 0x100 + ((ushort)local_c >> 8);
    }
    *(undefined2 *)((int)param_1 + 0x10a) = 1;
LAB_004ae668:
    iVar3 = Zlib_FlushWithCRC(param_1,0);
    if (iVar3 == 0) {
      PNG_SetSPLT((int)param_1,param_2,param_1[0x57],(uint)*(ushort *)((int)param_1 + 0x10a),
                  param_1 + 0x58);
    }
  }
  return;
}

