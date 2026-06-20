
/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined1 * __cdecl CRT_cftof(undefined4 *param_1,undefined1 *param_2,size_t param_3)

{
  uint unaff_retaddr;
  char local_30 [24];
  int local_18;
  int local_14;
  uint local_8;
  
  local_8 = DAT_004fce90 ^ unaff_retaddr;
  CRT_DecodeDoubleToString(*param_1,param_1[1],&local_18,local_30);
  CRT_FormatIntegerToString(param_2 + (local_18 == 0x2d),local_14 + param_3,(int)&local_18);
  CRT_FormatFractional(param_2,param_3,'\0');
  return param_2;
}

