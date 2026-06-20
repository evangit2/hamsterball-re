
/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void __cdecl CRT_DecodeDouble80(_CRT_DOUBLE *param_1,byte *param_2)

{
  uint unaff_retaddr;
  _LDBL12 local_18;
  int local_c;
  uint local_8;
  
  local_8 = DAT_004fce90 ^ unaff_retaddr;
  CRT_ParseExponent((undefined2 *)&local_18,&local_c,param_2,0,0,0,0);
  FID_conflict___ld12tod(&local_18,param_1);
  return;
}

