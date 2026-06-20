
undefined2 LicenseKey_Validate(byte *param_1,byte *param_2,byte *param_3,char *param_4)

{
  byte bVar1;
  byte *pbVar2;
  int iVar3;
  uint uVar4;
  byte *pbVar5;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  byte *pbVar6;
  undefined4 extraout_EDX;
  undefined4 extraout_EDX_00;
  char local_54 [43];
  byte local_29 [20];
  byte local_15;
  undefined1 local_14 [4];
  byte abStack_10 [4];
  int local_c;
  char local_8;
  
  if (param_1 != (byte *)0x0) {
    bVar1 = *param_1;
    pbVar5 = param_1;
    while (bVar1 != 0) {
      pbVar5 = pbVar5 + 1;
      bVar1 = *pbVar5;
    }
    local_15 = *pbVar5;
    pbVar2 = local_29 + 0x13;
    pbVar5 = pbVar5 + -1;
    do {
      pbVar6 = pbVar5;
      if (pbVar6 < param_1) {
        return 0;
      }
      pbVar5 = pbVar6 + -1;
      if (s__004f76b8[*pbVar6] != 0x20) {
        *pbVar2 = (&DAT_004f7698)[(short)s__004f76b8[*pbVar6]];
        pbVar2 = pbVar2 + -1;
      }
    } while (local_29 <= pbVar2);
    if (pbVar5 < param_1) {
      *pbVar2 = 0;
      pbVar2 = pbVar2 + -1;
    }
    else {
      pbVar6 = pbVar6 + -2;
      if (*pbVar5 != 0x2d) {
        return 0;
      }
      *pbVar2 = 0;
      for (; pbVar2 = pbVar2 + -1, param_1 <= pbVar6; pbVar6 = pbVar6 + -1) {
        *pbVar2 = *pbVar6;
      }
    }
    iVar3 = Base32_Decode(local_14,(char *)local_29,0x14);
    if (iVar3 != 0) {
      iVar3 = LicenseKey_ComputeHash(extraout_ECX,extraout_EDX,pbVar2 + 1,param_2,param_3);
      LicenseKey_EncryptBlock((undefined4 *)local_14,(undefined4 *)&DAT_004f7634,(ushort)iVar3);
      if (local_8 == '\0') {
        uVar4 = Hash_MixStringUpper(extraout_ECX_00,extraout_EDX_00,stack0xffffffee,param_2,param_3)
        ;
        NoiseTable_Init((ushort)uVar4);
        iVar3 = StrLen_Delay8((char *)(pbVar2 + 1));
        if (iVar3 == local_c) {
          if (param_4 == (char *)0x0) {
            return local_14._0_2_;
          }
          if (*param_4 == '\0') {
            return local_14._0_2_;
          }
          iVar3 = StrLen(param_4);
          if ((short)iVar3 == 5) {
            Base32_Encode(abStack_10,local_54,5);
            iVar3 = StrNEq(param_4,local_54,5);
            if (iVar3 != 0) {
              return local_14._0_2_;
            }
          }
        }
      }
    }
  }
  return 0;
}

