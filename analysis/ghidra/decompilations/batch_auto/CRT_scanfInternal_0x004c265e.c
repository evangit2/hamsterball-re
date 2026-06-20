
/* WARNING: Function: __SEH_prolog replaced with injection: SEH_prolog */
/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */
/* WARNING: Function: __SEH_epilog replaced with injection: EH_epilog3 */
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */

int __cdecl CRT_scanfInternal(FILE *param_1,byte *param_2,undefined4 *param_3)

{
  uint uVar1;
  byte bVar2;
  undefined1 *puVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  uint extraout_ECX;
  FILE *extraout_ECX_00;
  FILE *extraout_ECX_01;
  FILE *pFVar9;
  FILE *extraout_ECX_02;
  FILE *extraout_ECX_03;
  undefined4 extraout_ECX_04;
  uint extraout_ECX_05;
  byte bVar10;
  byte bVar11;
  wchar_t *pwVar12;
  byte *pbVar13;
  char *pcVar14;
  char *pcVar15;
  wchar_t *pwVar16;
  byte *pbVar17;
  bool bVar18;
  uint unaff_retaddr;
  wchar_t local_1dc [2];
  byte local_1d8;
  undefined1 local_1d7;
  uint local_1d4;
  byte *local_1d0;
  char local_1cc;
  char local_1cb [351];
  uint local_6c;
  wchar_t *local_68;
  undefined4 *local_64;
  undefined8 local_60;
  int local_58;
  char local_53;
  char local_52;
  char local_51;
  char local_50;
  char local_4f;
  char local_4e;
  byte local_4d;
  int local_4c;
  FILE *local_48;
  int local_44;
  byte local_3d;
  int local_3c;
  int local_38;
  int local_34;
  char local_2d;
  uint local_2c;
  int local_28;
  undefined1 *local_24;
  uint local_20;
  undefined1 *local_1c;
  undefined4 uStack_c;
  undefined *local_8;
  
  local_8 = &DAT_004ea408;
  uStack_c = 0x4c266d;
  local_20 = DAT_004fce90 ^ unaff_retaddr;
  local_24 = (undefined1 *)0x0;
  local_28 = 0;
  local_2c = 0;
  local_2d = '\0';
  local_34 = 0;
  local_38 = 0;
LAB_004c268c:
  while (*param_2 != 0) {
    uVar4 = (uint)*param_2;
    iVar5 = _isspace(uVar4);
    if (iVar5 == 0) {
      if (*param_2 != 0x25) {
        local_34 = local_34 + 1;
        uVar4 = __inc(uVar4,param_1);
        local_2c = uVar4;
        if (*param_2 != uVar4) goto LAB_004c309b;
        pbVar17 = param_2 + 1;
        if ((PTR_DAT_004fd060[(uVar4 & 0xff) * 2 + 1] & 0x80) != 0) {
          local_34 = local_34 + 1;
          uVar6 = __inc(PTR_DAT_004fd060,param_1);
          if (param_2[1] != uVar6) {
            if (uVar6 != 0xffffffff) {
              FID_conflict__ungetc(uVar6,param_1);
            }
            goto LAB_004c309b;
          }
          local_34 = local_34 + -1;
          pbVar17 = param_2 + 2;
        }
        goto LAB_004c30af;
      }
      iVar5 = 0;
      local_3c = 0;
      local_3d = 0;
      local_44 = 0;
      local_48 = (FILE *)0x0;
      local_4c = 0;
      local_4d = 0;
      local_4e = '\0';
      local_4f = '\0';
      local_50 = '\0';
      local_51 = '\0';
      local_52 = '\0';
      local_53 = '\x01';
      local_58 = 0;
      pbVar17 = param_2;
      do {
        pbVar13 = pbVar17 + 1;
        uVar6 = (uint)*pbVar13;
        uVar4 = uVar6;
        iVar7 = _isdigit(uVar6);
        param_2 = pbVar13;
        if (iVar7 == 0) {
          if (uVar6 < 0x4f) {
            if (uVar6 != 0x4e) {
              if (uVar6 == 0x2a) {
                local_4f = local_4f + '\x01';
              }
              else if (uVar6 != 0x46) {
                if (uVar6 == 0x49) {
                  bVar11 = pbVar17[2];
                  uVar4 = CONCAT31((int3)(uVar4 >> 8),bVar11);
                  if ((bVar11 == 0x36) && (pbVar17[3] == 0x34)) {
                    local_58 = local_58 + 1;
                    local_60 = 0;
                    param_2 = pbVar17 + 3;
                  }
                  else if (((bVar11 != 0x33) || (param_2 = pbVar17 + 3, *param_2 != 0x32)) &&
                          ((((param_2 = pbVar13, bVar11 != 100 && (bVar11 != 0x69)) &&
                            (bVar11 != 0x6f)) && ((bVar11 != 0x78 && (bVar11 != 0x58))))))
                  goto LAB_004c27cd;
                }
                else if (uVar6 == 0x4c) {
                  local_53 = local_53 + '\x01';
                }
                else {
LAB_004c27cd:
                  local_50 = local_50 + '\x01';
                  param_2 = pbVar13;
                }
              }
            }
          }
          else if (uVar6 == 0x68) {
            local_53 = local_53 + -1;
            local_52 = local_52 + -1;
          }
          else {
            if (uVar6 == 0x6c) {
              local_53 = local_53 + '\x01';
            }
            else if (uVar6 != 0x77) goto LAB_004c27cd;
            local_52 = local_52 + '\x01';
          }
        }
        else {
          local_48 = (FILE *)((int)&local_48->_ptr + 1);
          iVar5 = (uVar6 - 0x30) + iVar5 * 10;
        }
        pbVar17 = param_2;
      } while (local_50 == '\0');
      if (local_4f == '\0') {
        local_64 = param_3;
        local_68 = (wchar_t *)*param_3;
        param_3 = param_3 + 1;
      }
      pwVar12 = local_68;
      local_50 = '\0';
      if (local_52 == '\0') {
        if ((*param_2 == 0x53) || (*param_2 == 0x43)) {
          local_52 = '\x01';
        }
        else {
          local_52 = -1;
        }
      }
      uVar6 = *param_2 | 0x20;
      local_6c = uVar6;
      local_4c = iVar5;
      uVar8 = local_2c;
      if (uVar6 != 0x6e) {
        if ((uVar6 == 99) || (uVar6 == 0x7b)) {
          local_34 = local_34 + 1;
          uVar8 = __inc(uVar4,param_1);
        }
        else {
          do {
            local_34 = local_34 + 1;
            uVar8 = __inc(uVar4,param_1);
            uVar4 = uVar8;
            iVar5 = _isspace(uVar8);
          } while (iVar5 != 0);
        }
      }
      local_2c = uVar8;
      pFVar9 = local_48;
      uVar4 = local_2c;
      if ((local_48 == (FILE *)0x0) || (local_4c != 0)) {
        if (uVar6 < 0x70) {
          if (uVar6 == 0x6f) {
LAB_004c2e0d:
            if (local_2c == 0x2d) {
              local_4e = '\x01';
            }
            else if (local_2c != 0x2b) goto LAB_004c2e3e;
            local_4c = local_4c + -1;
            if ((local_4c == 0) && (local_48 != (FILE *)0x0)) {
              local_50 = '\x01';
            }
            else {
              local_34 = local_34 + 1;
              local_2c = __inc(local_48,param_1);
            }
            goto LAB_004c2e3e;
          }
          if (uVar6 == 99) {
            if (local_48 == (FILE *)0x0) {
              local_48 = (FILE *)0x1;
              local_4c = local_4c + 1;
            }
LAB_004c2a87:
            if ('\0' < local_52) {
              local_51 = '\x01';
            }
            goto LAB_004c2c12;
          }
          if (uVar6 == 100) goto LAB_004c2e0d;
          if (uVar6 < 0x65) {
LAB_004c2ac0:
            if (*param_2 != local_2c) goto LAB_004c309b;
            local_2d = local_2d + -1;
            if (local_4f == '\0') {
              param_3 = local_64;
            }
            goto LAB_004c303f;
          }
          if (0x67 < uVar6) {
            if (uVar6 == 0x69) {
              uVar6 = 100;
              goto LAB_004c28d7;
            }
            if (uVar6 != 0x6e) goto LAB_004c2ac0;
            iVar5 = local_34;
            if (local_4f != '\0') goto LAB_004c303f;
            goto LAB_004c301f;
          }
          pcVar14 = &local_1cc;
          if (local_2c == 0x2d) {
            local_1cc = '-';
            pcVar14 = local_1cb;
LAB_004c290d:
            local_4c = local_4c + -1;
            local_34 = local_34 + 1;
            local_2c = __inc(local_48,param_1);
          }
          else if (local_2c == 0x2b) goto LAB_004c290d;
          if ((local_48 == (FILE *)0x0) || (0x15d < local_4c)) {
            local_4c = 0x15d;
          }
          while( true ) {
            uVar4 = local_2c;
            uVar6 = local_2c;
            iVar5 = _isdigit(local_2c);
            if ((iVar5 == 0) ||
               (iVar5 = local_4c + -1, bVar18 = local_4c == 0, local_4c = iVar5, bVar18)) break;
            local_44 = local_44 + 1;
            *pcVar14 = (char)uVar4;
            pcVar14 = pcVar14 + 1;
            local_34 = local_34 + 1;
            local_2c = __inc(uVar6,param_1);
          }
          if ((DAT_004fd084 == (char)uVar4) &&
             (iVar5 = local_4c + -1, bVar18 = local_4c != 0, local_4c = iVar5, bVar18)) {
            local_34 = local_34 + 1;
            uVar4 = __inc(uVar6,param_1);
            *pcVar14 = DAT_004fd084;
            while( true ) {
              pcVar14 = pcVar14 + 1;
              uVar6 = uVar4;
              local_2c = uVar4;
              iVar5 = _isdigit(uVar4);
              if ((iVar5 == 0) ||
                 (iVar5 = local_4c + -1, bVar18 = local_4c == 0, local_4c = iVar5, bVar18)) break;
              local_44 = local_44 + 1;
              *pcVar14 = (char)uVar4;
              local_34 = local_34 + 1;
              uVar4 = __inc(uVar6,param_1);
            }
          }
          pcVar15 = pcVar14;
          if ((local_44 != 0) &&
             (((uVar4 == 0x65 || (uVar4 == 0x45)) &&
              (iVar5 = local_4c + -1, bVar18 = local_4c != 0, local_4c = iVar5, bVar18)))) {
            *pcVar14 = 'e';
            pcVar15 = pcVar14 + 1;
            local_34 = local_34 + 1;
            uVar4 = __inc(uVar6,param_1);
            local_2c = uVar4;
            if (uVar4 == 0x2d) {
              *pcVar15 = '-';
              pcVar15 = pcVar14 + 2;
LAB_004c29f7:
              bVar18 = local_4c != 0;
              uVar6 = extraout_ECX;
              local_4c = local_4c + -1;
              if (bVar18) goto LAB_004c2a16;
              local_4c = 0;
            }
            else if (uVar4 == 0x2b) goto LAB_004c29f7;
            while ((uVar6 = uVar4, iVar5 = _isdigit(uVar4), iVar5 != 0 &&
                   (iVar5 = local_4c + -1, bVar18 = local_4c != 0, local_4c = iVar5, bVar18))) {
              local_44 = local_44 + 1;
              *pcVar15 = (char)uVar4;
              pcVar15 = pcVar15 + 1;
LAB_004c2a16:
              local_34 = local_34 + 1;
              uVar4 = __inc(uVar6,param_1);
              local_2c = uVar4;
            }
          }
          local_34 = local_34 + -1;
          if (uVar4 != 0xffffffff) {
            FID_conflict__ungetc(uVar4,param_1);
          }
          if (local_44 != 0) {
            if (local_4f == '\0') {
              local_38 = local_38 + 1;
              *pcVar15 = '\0';
              (*(code *)PTR_CRT_amsg_exit_004fc818)(local_53 + -1,local_68,&local_1cc);
            }
            goto LAB_004c303f;
          }
        }
        else {
          if (uVar6 == 0x70) {
            local_53 = '\x01';
            goto LAB_004c2e0d;
          }
          if (uVar6 == 0x73) goto LAB_004c2a87;
          if (uVar6 == 0x75) goto LAB_004c2e0d;
          if (uVar6 != 0x78) {
            if (uVar6 == 0x7b) {
              if ('\0' < local_52) {
                local_51 = '\x01';
              }
              pbVar13 = param_2 + 1;
              pbVar17 = pbVar13;
              if (*pbVar13 == 0x5e) {
                pbVar17 = param_2 + 2;
                local_4d = 0xff;
              }
              if (local_24 == (undefined1 *)0x0) {
                local_1c = &stack0xfffffe18;
                local_24 = &stack0xfffffe18;
                local_8 = (undefined *)0xffffffff;
              }
              puVar3 = local_24;
              local_1d0 = pbVar17;
              _memset(local_24,0,0x20);
              pFVar9 = extraout_ECX_00;
              bVar11 = local_3d;
              if ((local_6c == 0x7b) && (*pbVar17 == 0x5d)) {
                puVar3[0xb] = 0x20;
                pbVar17 = pbVar17 + 1;
                bVar11 = 0x5d;
              }
LAB_004c2bf5:
              do {
                bVar10 = *pbVar17;
                if (bVar10 == 0x5d) goto code_r0x004c2c03;
                if ((bVar10 == 0x2d) && (bVar11 != 0)) {
                  bVar2 = pbVar17[1];
                  pFVar9 = (FILE *)CONCAT31((int3)((uint)pFVar9 >> 8),bVar2);
                  if (bVar2 != 0x5d) {
                    bVar10 = bVar2;
                    if (bVar11 < bVar2) {
                      bVar10 = bVar11;
                      bVar11 = bVar2;
                    }
                    if (bVar10 <= bVar11) {
                      uVar4 = (uint)bVar10;
                      local_1d4 = (uint)(byte)((bVar11 - bVar10) + 1);
                      do {
                        pFVar9 = (FILE *)(uVar4 & 7);
                        puVar3[uVar4 >> 3] = puVar3[uVar4 >> 3] | '\x01' << (sbyte)pFVar9;
                        uVar4 = uVar4 + 1;
                        local_1d4 = local_1d4 - 1;
                      } while (local_1d4 != 0);
                    }
                    pbVar17 = pbVar17 + 2;
                    bVar11 = 0;
                    goto LAB_004c2bf5;
                  }
                }
                local_3d = bVar10;
                pFVar9 = (FILE *)(bVar10 & 7);
                puVar3[bVar10 >> 3] = puVar3[bVar10 >> 3] | '\x01' << (sbyte)pFVar9;
                pbVar17 = pbVar17 + 1;
                bVar11 = local_3d;
              } while( true );
            }
            goto LAB_004c2ac0;
          }
LAB_004c28d7:
          if (local_2c == 0x2d) {
            local_4e = '\x01';
LAB_004c2d0b:
            local_4c = local_4c + -1;
            if ((local_4c == 0) && (local_48 != (FILE *)0x0)) {
              local_50 = '\x01';
            }
            else {
              local_34 = local_34 + 1;
              local_2c = __inc(local_48,param_1);
              pFVar9 = extraout_ECX_03;
            }
          }
          else if (local_2c == 0x2b) goto LAB_004c2d0b;
          if (local_2c == 0x30) {
            local_34 = local_34 + 1;
            local_2c = __inc(pFVar9,param_1);
            if (((char)local_2c == 'x') || ((char)local_2c == 'X')) {
              local_34 = local_34 + 1;
              local_2c = __inc(extraout_ECX_04,param_1);
              if ((local_48 != (FILE *)0x0) && (local_4c = local_4c + -2, local_4c < 1)) {
                local_50 = local_50 + '\x01';
              }
              uVar6 = 0x78;
            }
            else {
              local_44 = 1;
              if (uVar6 == 0x78) {
                local_34 = local_34 + -1;
                if (local_2c != 0xffffffff) {
                  FID_conflict__ungetc(local_2c,param_1);
                }
                local_2c = 0x30;
              }
              else {
                if ((local_48 != (FILE *)0x0) && (local_4c = local_4c + -1, local_4c == 0)) {
                  local_50 = local_50 + '\x01';
                }
                uVar6 = 0x6f;
              }
            }
          }
LAB_004c2e3e:
          uVar4 = local_2c;
          if (local_58 == 0) {
            while (local_50 == '\0') {
              uVar8 = uVar4;
              if ((uVar6 == 0x78) || (uVar6 == 0x70)) {
                iVar5 = _isxdigit(uVar4);
                if (iVar5 != 0) {
                  local_3c = local_3c << 4;
                  uVar8 = uVar4;
                  iVar5 = _isdigit(uVar4);
                  if (iVar5 == 0) {
                    uVar4 = (uVar4 & 0xffffffdf) - 7;
                  }
                  goto LAB_004c2fa4;
                }
LAB_004c2fa1:
                local_50 = local_50 + '\x01';
              }
              else {
                iVar5 = _isdigit(uVar4);
                if (iVar5 == 0) goto LAB_004c2fa1;
                if (uVar6 == 0x6f) {
                  if (0x37 < (int)uVar4) goto LAB_004c2fa1;
                  local_3c = local_3c << 3;
                }
                else {
                  local_3c = local_3c * 10;
                }
              }
LAB_004c2fa4:
              if (local_50 == '\0') {
                local_44 = local_44 + 1;
                local_3c = local_3c + -0x30 + uVar4;
                if ((local_48 == (FILE *)0x0) || (local_4c = local_4c + -1, local_4c != 0)) {
                  local_34 = local_34 + 1;
                  uVar4 = __inc(uVar8,param_1);
                }
                else {
                  local_50 = '\x01';
                }
              }
              else {
                local_34 = local_34 + -1;
                if (uVar4 != 0xffffffff) {
                  FID_conflict__ungetc(uVar4,param_1);
                }
              }
            }
            local_2c = uVar4;
            if (local_4e != '\0') {
              local_3c = -local_3c;
            }
          }
          else {
            while (local_50 == '\0') {
              uVar8 = uVar4;
              if ((uVar6 == 0x78) || (uVar6 == 0x70)) {
                iVar5 = _isxdigit(uVar4);
                if (iVar5 != 0) {
                  local_60 = CONCAT44(local_60._4_4_ << 4 | (uint)local_60 >> 0x1c,
                                      (uint)local_60 << 4);
                  uVar8 = uVar4;
                  iVar5 = _isdigit(uVar4);
                  if (iVar5 == 0) {
                    uVar4 = (uVar4 & 0xffffffdf) - 7;
                  }
                  goto LAB_004c2ed1;
                }
LAB_004c2ece:
                local_50 = local_50 + '\x01';
              }
              else {
                iVar5 = _isdigit(uVar4);
                if (iVar5 == 0) goto LAB_004c2ece;
                if (uVar6 == 0x6f) {
                  if (0x37 < (int)uVar4) goto LAB_004c2ece;
                  uVar8 = local_60._4_4_ << 3 | (uint)local_60 >> 0x1d;
                  local_60 = CONCAT44(uVar8,(uint)local_60 << 3);
                }
                else {
                  local_60 = __allmul((uint)local_60,local_60._4_4_,10,0);
                  uVar8 = extraout_ECX_05;
                }
              }
LAB_004c2ed1:
              if (local_50 == '\0') {
                local_44 = local_44 + 1;
                uVar1 = uVar4 - 0x30;
                local_60 = CONCAT44(local_60._4_4_ + ((int)uVar1 >> 0x1f) +
                                    (uint)CARRY4((uint)local_60,uVar1),(uint)local_60 + uVar1);
                if ((local_48 == (FILE *)0x0) || (local_4c = local_4c + -1, local_4c != 0)) {
                  local_34 = local_34 + 1;
                  uVar4 = __inc(uVar8,param_1);
                }
                else {
                  local_50 = '\x01';
                }
              }
              else {
                local_34 = local_34 + -1;
                if (uVar4 != 0xffffffff) {
                  FID_conflict__ungetc(uVar4,param_1);
                }
              }
            }
            local_2c = uVar4;
            if (local_4e != '\0') {
              local_60._4_4_ = (int)((ulonglong)local_60 >> 0x20);
              local_60 = CONCAT44(-(local_60._4_4_ + (uint)((uint)local_60 != 0)),-(uint)local_60);
            }
          }
          if (uVar6 == 0x46) {
            local_44 = 0;
          }
          if (local_44 != 0) {
            if (local_4f == '\0') {
              local_38 = local_38 + 1;
              iVar5 = local_3c;
              pwVar12 = local_68;
LAB_004c301f:
              if (local_58 == 0) {
                if (local_53 == '\0') {
                  *pwVar12 = (wchar_t)iVar5;
                }
                else {
                  *(int *)pwVar12 = iVar5;
                }
              }
              else {
                *(uint *)pwVar12 = (uint)local_60;
                *(int *)(pwVar12 + 2) = local_60._4_4_;
              }
            }
            goto LAB_004c303f;
          }
        }
      }
      else {
LAB_004c309b:
        if (uVar4 != 0xffffffff) {
          FID_conflict__ungetc(local_2c,param_1);
        }
      }
      break;
    }
    local_34 = local_34 + -1;
    do {
      local_34 = local_34 + 1;
      uVar6 = __inc(uVar4,param_1);
      uVar4 = uVar6;
      iVar5 = _isspace(uVar6);
    } while (iVar5 != 0);
    if (uVar6 != 0xffffffff) {
      FID_conflict__ungetc(uVar6,param_1);
    }
    do {
      param_2 = param_2 + 1;
      iVar5 = _isspace((uint)*param_2);
    } while (iVar5 != 0);
  }
  goto LAB_004c30cb;
code_r0x004c2c03:
  pwVar12 = local_68;
  uVar6 = local_6c;
  param_2 = pbVar13;
  if (local_6c == 0x7b) {
    param_2 = pbVar17;
  }
LAB_004c2c12:
  local_34 = local_34 + -1;
  pwVar16 = pwVar12;
  if (local_2c != 0xffffffff) {
    pFVar9 = param_1;
    FID_conflict__ungetc(local_2c,param_1);
  }
  while( true ) {
    if ((local_48 != (FILE *)0x0) &&
       (iVar5 = local_4c + -1, bVar18 = local_4c == 0, local_4c = iVar5, bVar18)) goto LAB_004c2dd0;
    local_34 = local_34 + 1;
    local_2c = __inc(pFVar9,param_1);
    if ((local_2c == 0xffffffff) ||
       ((bVar11 = (byte)local_2c, pFVar9 = extraout_ECX_01, uVar6 != 99 &&
        (((uVar6 != 0x73 || (((8 < (int)local_2c && ((int)local_2c < 0xe)) || (local_2c == 0x20))))
         && ((uVar6 != 0x7b ||
             (pFVar9 = (FILE *)(int)(char)(local_24[(int)local_2c >> 3] ^ local_4d),
             uVar6 = local_6c, ((uint)pFVar9 & 1 << (bVar11 & 7)) == 0)))))))) break;
    if (local_4f == '\0') {
      if (local_51 == '\0') {
        *(byte *)pwVar12 = bVar11;
        pwVar12 = (wchar_t *)((int)pwVar12 + 1);
        local_68 = pwVar12;
      }
      else {
        local_1d8 = bVar11;
        if ((PTR_DAT_004fd060[(local_2c & 0xff) * 2 + 1] & 0x80) != 0) {
          local_34 = local_34 + 1;
          uVar4 = __inc(PTR_DAT_004fd060,param_1);
          local_1d7 = (undefined1)uVar4;
        }
        _mbtowc(local_1dc,(char *)&local_1d8,DAT_004fd080);
        *pwVar12 = local_1dc[0];
        pwVar12 = pwVar12 + 1;
        pFVar9 = extraout_ECX_02;
        local_68 = pwVar12;
      }
    }
    else {
      pwVar16 = (wchar_t *)((int)pwVar16 + 1);
    }
  }
  local_34 = local_34 + -1;
  if (local_2c != 0xffffffff) {
    FID_conflict__ungetc(local_2c,param_1);
  }
LAB_004c2dd0:
  if (pwVar16 == pwVar12) {
LAB_004c30cb:
    if (local_28 == 1) {
      _free(local_24);
    }
    if (((local_2c == 0xffffffff) && (local_38 == 0)) && (local_2d == '\0')) {
      local_38 = -1;
    }
    return local_38;
  }
  if ((local_4f == '\0') && (local_38 = local_38 + 1, local_6c != 99)) {
    if (local_51 == '\0') {
      *(byte *)local_68 = 0;
    }
    else {
      *local_68 = L'\0';
    }
  }
LAB_004c303f:
  local_2d = local_2d + '\x01';
  pbVar17 = param_2 + 1;
LAB_004c30af:
  param_2 = pbVar17;
  if ((local_2c == 0xffffffff) && ((*param_2 != 0x25 || (param_2[1] != 0x6e)))) goto LAB_004c30cb;
  goto LAB_004c268c;
}

