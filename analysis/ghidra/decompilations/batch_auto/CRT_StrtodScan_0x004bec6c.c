
/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void __cdecl CRT_StrtodScan(uint *param_1,byte *param_2)

{
  uint uVar1;
  INTRNCVT_STATUS IVar2;
  uint uVar3;
  uint unaff_retaddr;
  _LDBL12 local_20;
  _CRT_DOUBLE local_14;
  int local_c;
  uint local_8;
  
  local_8 = DAT_004fce90 ^ unaff_retaddr;
  uVar3 = 0;
  uVar1 = CRT_ParseExponent((undefined2 *)&local_20,&local_c,param_2,0,0,0,0);
  if ((uVar1 & 4) == 0) {
    IVar2 = FID_conflict___ld12tod(&local_20,&local_14);
    if (((uVar1 & 2) != 0) || (IVar2 == INTRNCVT_OVERFLOW)) {
      uVar3 = 0x80;
    }
    if (((uVar1 & 1) != 0) || (IVar2 == INTRNCVT_UNDERFLOW)) {
      uVar3 = uVar3 | 0x100;
    }
  }
  else {
    uVar3 = 0x200;
    local_14.x._0_4_ = 0;
    local_14.x._4_4_ = 0;
  }
  *param_1 = uVar3;
  param_1[1] = local_c - (int)param_2;
  param_1[4] = local_14.x._0_4_;
  param_1[5] = local_14.x._4_4_;
  return;
}

