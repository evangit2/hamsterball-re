
int __fastcall
LicenseKey_ComputeHash
          (undefined4 param_1,undefined4 param_2,byte *param_3,byte *param_4,byte *param_5)

{
  uint uVar1;
  undefined4 extraout_ECX;
  undefined4 extraout_EDX;
  
  uVar1 = Hash_MixString(param_1,param_2,0,param_3);
  uVar1 = Hash_MixStringUpper(extraout_ECX,extraout_EDX,uVar1,param_4,param_5);
  return (uVar1 & 0xffff7fff) + 0x1777;
}

