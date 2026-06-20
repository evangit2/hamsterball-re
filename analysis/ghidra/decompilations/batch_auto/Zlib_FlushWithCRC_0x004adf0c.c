
undefined4 __cdecl Zlib_FlushWithCRC(int *param_1,uint param_2)

{
  uint uVar1;
  bool bVar2;
  undefined3 extraout_var;
  undefined4 uVar3;
  
  uVar1 = param_1[0x28];
  for (; uVar1 < param_2; param_2 = param_2 - uVar1) {
    Zlib_UpdateHash(param_1,(byte *)param_1[0x27],param_1[0x28]);
  }
  if (param_2 != 0) {
    Zlib_UpdateHash(param_1,(byte *)param_1[0x27],param_2);
  }
  bVar2 = Zlib_VerifyChecksum(param_1);
  if (CONCAT31(extraout_var,bVar2) == 0) {
    uVar3 = 0;
  }
  else {
    if ((((*(byte *)(param_1 + 0x43) & 0x20) == 0) || ((*(byte *)((int)param_1 + 0x5d) & 2) != 0))
       && (((*(byte *)(param_1 + 0x43) & 0x20) != 0 || ((*(byte *)((int)param_1 + 0x5d) & 4) == 0)))
       ) {
      EH_MediaParser_longjmp(param_1,(undefined4 *)"CRC error");
    }
    else {
      EH_MediaParser_seh((int)param_1,(undefined4 *)"CRC error");
    }
    uVar3 = 1;
  }
  return uVar3;
}

