
/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void __cdecl CRT_cftoe(undefined4 *param_1,undefined1 *param_2,size_t param_3,int param_4)

{
  char *pcVar1;
  int iVar2;
  char *pcVar3;
  uint unaff_retaddr;
  char local_30 [24];
  int local_18;
  int local_14;
  uint local_8;
  
  local_8 = DAT_004fce90 ^ unaff_retaddr;
  CRT_DecodeDoubleToString(*param_1,param_1[1],&local_18,local_30);
  iVar2 = local_14 + -1;
  CRT_FormatIntegerToString(param_2 + (local_18 == 0x2d),param_3,(int)&local_18);
  local_14 = local_14 + -1;
  if ((local_14 < -4) || ((int)param_3 <= local_14)) {
    __cftoe2(param_3,param_4,'\x01');
  }
  else {
    pcVar1 = param_2 + (local_18 == 0x2d);
    if (iVar2 < local_14) {
      do {
        pcVar3 = pcVar1;
        pcVar1 = pcVar3 + 1;
      } while (*pcVar3 != '\0');
      pcVar3[-1] = '\0';
    }
    CRT_FormatFractional(param_2,param_3,'\x01');
  }
  return;
}

