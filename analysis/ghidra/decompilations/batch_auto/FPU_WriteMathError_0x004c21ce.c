
/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void __cdecl
FPU_WriteMathError(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3,int param_4)

{
  int iVar1;
  int *piVar2;
  uint unaff_retaddr;
  undefined4 local_28;
  char *local_24;
  undefined8 local_20;
  undefined8 local_18;
  undefined8 local_10;
  uint local_8;
  
  local_8 = DAT_004fce90 ^ unaff_retaddr;
  if (0xa1 < param_4) {
    if (param_4 < 0x3eb) {
      if (param_4 != 0x3ea) {
        if (param_4 == 0xa2) {
          local_28 = 4;
          goto LAB_004c2383;
        }
        if (param_4 == 0xa6) {
          local_28 = 3;
          local_24 = "exp10";
        }
        else {
          if (param_4 != 0xaa) {
            if (param_4 == 0xab) {
              local_24 = "log2";
              goto LAB_004c241c;
            }
            if (param_4 == 1000) {
              local_24 = "log";
            }
            else {
              if (param_4 != 0x3e9) {
                return;
              }
              local_24 = "log10";
            }
            goto LAB_004c2412;
          }
          local_28 = 2;
          local_24 = "log2";
        }
        goto LAB_004c238a;
      }
      local_24 = "exp";
    }
    else if (param_4 == 0x3eb) {
      local_24 = "atan";
    }
    else if (param_4 == 0x3ec) {
      local_24 = "ceil";
    }
    else if (param_4 == 0x3ed) {
      local_24 = "floor";
    }
    else {
      if (param_4 == 0x3ee) goto LAB_004c23ea;
      if (param_4 != 0x3ef) {
        return;
      }
      local_24 = "modf";
    }
LAB_004c2412:
    *param_3 = *param_1;
    goto LAB_004c241c;
  }
  if (param_4 == 0xa1) {
    local_28 = 3;
LAB_004c2383:
    local_24 = "exp2";
    goto LAB_004c238a;
  }
  if (param_4 < 0x19) {
    if (param_4 == 0x18) {
      local_28 = 3;
      goto LAB_004c227b;
    }
    if (param_4 == 2) {
      local_28 = 2;
      local_24 = "log";
    }
    else {
      if (param_4 == 3) {
        local_24 = "log";
LAB_004c241c:
        local_20 = *param_1;
        local_28 = 1;
        local_18 = *param_2;
        local_10 = *param_3;
        iVar1 = (*(code *)PTR_ReturnZero_004fce88)(&local_28);
        if (iVar1 == 0) {
          piVar2 = __errno();
          *piVar2 = 0x21;
        }
        goto LAB_004c244f;
      }
      if (param_4 == 8) {
        local_28 = 2;
        local_24 = "log10";
      }
      else {
        if (param_4 == 9) {
          local_24 = "log10";
          goto LAB_004c241c;
        }
        if (param_4 != 0xe) {
          if (param_4 != 0xf) {
            return;
          }
          local_24 = "exp";
          goto LAB_004c22c8;
        }
        local_28 = 3;
        local_24 = "exp";
      }
    }
  }
  else {
    if (param_4 == 0x19) {
      local_24 = "pow";
LAB_004c22c8:
      local_20 = *param_1;
      local_18 = *param_2;
      local_10 = *param_3;
      local_28 = 4;
      (*(code *)PTR_ReturnZero_004fce88)(&local_28);
      goto LAB_004c244f;
    }
    if (param_4 == 0x1a) {
      *param_3 = 0x3ff0000000000000;
      return;
    }
    if (param_4 != 0x1b) {
      if (param_4 != 0x1c) {
        if (param_4 != 0x1d) {
          return;
        }
        local_24 = "pow";
        goto LAB_004c2412;
      }
LAB_004c23ea:
      local_24 = "pow";
      goto LAB_004c241c;
    }
    local_28 = 2;
LAB_004c227b:
    local_24 = "pow";
  }
LAB_004c238a:
  local_20 = *param_1;
  local_18 = *param_2;
  local_10 = *param_3;
  iVar1 = (*(code *)PTR_ReturnZero_004fce88)(&local_28);
  if (iVar1 == 0) {
    piVar2 = __errno();
    *piVar2 = 0x22;
  }
LAB_004c244f:
  *param_3 = local_10;
  return;
}

