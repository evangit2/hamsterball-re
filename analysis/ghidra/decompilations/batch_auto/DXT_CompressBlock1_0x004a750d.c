
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __cdecl DXT_CompressBlock1(ushort *param_1,float param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  int in_EAX;
  float *pfVar4;
  uint uVar5;
  uint uVar6;
  float extraout_EDX;
  float extraout_EDX_00;
  float fVar7;
  ushort uVar8;
  int iVar9;
  ushort uVar10;
  int iVar11;
  float local_2d4 [64];
  float local_1d4;
  float local_1d0;
  undefined1 local_1cc [4];
  undefined1 local_1c8 [4];
  undefined1 local_1c4 [4];
  undefined1 local_1c0 [20];
  undefined1 local_1ac [4];
  undefined1 local_1a8 [4];
  undefined1 local_1a4 [4];
  undefined1 local_1a0 [4];
  undefined1 local_19c [4];
  undefined1 local_198 [4];
  undefined1 local_194 [4];
  float local_190;
  undefined1 local_18c [4];
  undefined1 local_188 [4];
  undefined1 local_184 [4];
  undefined1 local_180 [172];
  float local_d4;
  float local_d0;
  float local_cc;
  int local_c4;
  undefined1 *local_c0;
  undefined1 *local_bc;
  undefined1 *local_b8;
  undefined1 *local_b4;
  undefined1 *local_b0;
  undefined1 *local_ac;
  int local_a8;
  undefined1 *local_a4;
  undefined1 *local_a0;
  float local_9c [4];
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  undefined1 *local_5c;
  undefined1 *local_58;
  undefined1 *local_54;
  undefined1 *local_50;
  float local_4c;
  float local_48;
  float local_44;
  float fStack_40;
  float local_3c;
  float local_38;
  float local_34;
  undefined1 *local_2c;
  undefined1 *local_28;
  undefined *local_24;
  float local_20;
  float local_1c;
  float local_18;
  float fStack_14;
  float local_10;
  float *local_c;
  float local_8;
  
  if (param_2 == 0.0) {
    local_10 = 5.60519e-45;
  }
  else {
    iVar9 = 0;
    pfVar4 = (float *)(in_EAX + 0xc);
    iVar11 = 0x10;
    do {
      if (*pfVar4 < _DAT_004cf3f0) {
        iVar9 = iVar9 + 1;
      }
      pfVar4 = pfVar4 + 4;
      iVar11 = iVar11 + -1;
    } while (iVar11 != 0);
    if (iVar9 == 0x10) {
      param_1[1] = 0xffff;
      param_1[2] = 0xffff;
      param_1[3] = 0xffff;
      *param_1 = 0;
      return 0;
    }
    local_10 = (float)(4 - (uint)(iVar9 != 0));
  }
  pfVar4 = &local_1d4;
  for (iVar11 = 0x40; iVar11 != 0; iVar11 = iVar11 + -1) {
    *pfVar4 = 0.0;
    pfVar4 = pfVar4 + 1;
  }
  SSE2_SetFPControlWord(0);
  local_58 = local_1cc + -in_EAX;
  local_b8 = local_1c8 + -in_EAX;
  local_c0 = local_1c4 + -in_EAX;
  local_5c = local_1ac + -in_EAX;
  local_b4 = local_1a8 + -in_EAX;
  local_ac = local_1a4 + -in_EAX;
  local_28 = local_19c + -in_EAX;
  local_bc = local_198 + -in_EAX;
  local_a4 = local_194 + -in_EAX;
  local_2c = local_18c + -in_EAX;
  local_a0 = local_188 + -in_EAX;
  local_b0 = local_184 + -in_EAX;
  pfVar4 = (float *)(in_EAX + 8);
  local_a8 = (int)&local_1d4 - in_EAX;
  uVar5 = 0;
  param_2 = extraout_EDX;
  do {
    local_20 = *(float *)((int)&local_1d4 + uVar5) + pfVar4[-2];
    fVar7 = pfVar4[-1];
    fVar1 = *(float *)(local_1cc + (uVar5 - 4));
    local_1c = fVar7 + fVar1;
    local_18 = *(float *)(local_a8 + (int)pfVar4) + *pfVar4;
    local_c4 = (int)ROUND(local_20 * _DAT_004d5c24 + _DAT_004cf3f0);
    local_24 = (undefined *)((float)local_c4 * _DAT_004d5c50);
    *(undefined **)((int)local_2d4 + uVar5) = local_24;
    local_50 = (undefined1 *)(int)ROUND((fVar7 + fVar1) * _DAT_004dc064 + _DAT_004cf3f0);
    local_8 = (float)(int)local_50 * _DAT_004dc08c;
    *(float *)((int)local_2d4 + uVar5 + 4) = local_8;
    local_54 = (undefined1 *)(int)ROUND(local_18 * _DAT_004d5c24 + _DAT_004cf3f0);
    fVar7 = (float)(int)local_54 * _DAT_004d5c50;
    local_c = (float *)((uint)param_2 & 3);
    *(float *)(((int)local_2d4 - in_EAX) + (int)pfVar4) = fVar7;
    *(undefined4 *)((int)local_2d4 + (4 - in_EAX) + (int)pfVar4) = 0x3f800000;
    fVar3 = local_20 - (float)local_24;
    fVar2 = local_1c - local_8;
    local_38 = fVar2;
    fVar1 = local_18 - fVar7;
    local_34 = fVar1;
    if (((uint)param_2 & 3) != 3) {
      *(float *)(local_58 + (int)pfVar4) =
           fVar3 * _DAT_004e5a78 + *(float *)(local_58 + (int)pfVar4);
      *(float *)(local_b8 + (int)pfVar4) =
           fVar2 * _DAT_004e5a78 + *(float *)(local_b8 + (int)pfVar4);
      *(float *)(local_c0 + (int)pfVar4) =
           fVar1 * _DAT_004e5a78 + *(float *)(local_c0 + (int)pfVar4);
    }
    if (uVar5 < 0xc0) {
      if (local_c != (float *)0x0) {
        *(float *)(local_5c + (int)pfVar4) =
             fVar3 * _DAT_004e5a74 + *(float *)(local_5c + (int)pfVar4);
        *(float *)(local_b4 + (int)pfVar4) =
             fVar2 * _DAT_004e5a74 + *(float *)(local_b4 + (int)pfVar4);
        *(float *)(local_ac + (int)pfVar4) =
             fVar1 * _DAT_004e5a74 + *(float *)(local_ac + (int)pfVar4);
      }
      *(float *)(local_28 + (int)pfVar4) =
           fVar3 * _DAT_004e5a70 + *(float *)(local_28 + (int)pfVar4);
      *(float *)(local_bc + (int)pfVar4) =
           fVar2 * _DAT_004e5a70 + *(float *)(local_bc + (int)pfVar4);
      *(float *)(local_a4 + (int)pfVar4) =
           fVar1 * _DAT_004e5a70 + *(float *)(local_a4 + (int)pfVar4);
      if (local_c != (float *)0x3) {
        *(float *)(local_2c + (int)pfVar4) =
             fVar3 * _DAT_004db564 + *(float *)(local_2c + (int)pfVar4);
        *(float *)(local_a0 + (int)pfVar4) =
             fVar2 * _DAT_004db564 + *(float *)(local_a0 + (int)pfVar4);
        *(float *)(local_b0 + (int)pfVar4) =
             fVar1 * _DAT_004db564 + *(float *)(local_b0 + (int)pfVar4);
      }
    }
    param_2 = (float)((int)param_2 + 1);
    uVar6 = uVar5 + 0x10;
    *(float *)((int)local_2d4 + uVar5) = (float)local_24 * DAT_004fab40;
    *(float *)((int)local_2d4 + uVar5 + 4) = local_8 * DAT_004fab44;
    *(float *)(((int)local_2d4 - in_EAX) + (int)pfVar4) = fVar7 * DAT_004fab48;
    pfVar4 = pfVar4 + 4;
    uVar5 = uVar6;
  } while (uVar6 < 0x100);
  Color_FitPrincipalAxis(&local_4c,&local_20,(int)local_2d4,local_10);
  local_d4 = local_4c * _DAT_004fab50;
  local_d0 = local_48 * _DAT_004fab54;
  local_cc = local_44 * _DAT_004fab58;
  local_3c = local_20 * _DAT_004fab50;
  local_38 = local_1c * _DAT_004fab54;
  local_34 = local_18 * _DAT_004fab58;
  uVar5 = Color_EncodeRGB555(&local_d4);
  local_28 = (undefined1 *)uVar5;
  uVar6 = Color_EncodeRGB555(&local_3c);
  uVar8 = (ushort)uVar5;
  uVar10 = (ushort)uVar6;
  if ((local_10 == 5.60519e-45) && (uVar8 == uVar10)) {
    param_1[2] = 0;
    param_1[3] = 0;
    *param_1 = uVar8;
    param_1[1] = uVar10;
  }
  else {
    local_2c = (undefined1 *)uVar6;
    Color_DecodeRGB555(uVar5);
    Color_DecodeRGB555(uVar6);
    local_4c = local_d4 * DAT_004fab40;
    local_48 = local_d0 * DAT_004fab44;
    local_44 = local_cc * DAT_004fab48;
    local_20 = local_3c * DAT_004fab40;
    local_1c = local_38 * DAT_004fab44;
    local_18 = local_34 * DAT_004fab48;
    if ((local_10 == 4.2039e-45) == uVar8 <= uVar10) {
      *param_1 = uVar8;
      param_1[1] = uVar10;
      local_9c[0] = local_4c;
      local_9c[1] = local_48;
      local_9c[2] = local_44;
      local_9c[3] = fStack_40;
      pfVar4 = &local_20;
    }
    else {
      param_1[1] = uVar8;
      *param_1 = uVar10;
      local_9c[0] = local_20;
      local_9c[1] = local_1c;
      local_9c[2] = local_18;
      local_9c[3] = fStack_14;
      pfVar4 = &local_4c;
    }
    local_8c = *pfVar4;
    local_88 = pfVar4[1];
    local_84 = pfVar4[2];
    local_80 = pfVar4[3];
    local_20 = *pfVar4 - local_9c[0];
    if (local_10 == 4.2039e-45) {
      local_24 = &DAT_004e5a64;
      local_78 = (local_88 - local_9c[1]) * _DAT_004cf3f0;
      local_74 = (local_84 - local_9c[2]) * _DAT_004cf3f0;
      local_70 = (local_80 - local_9c[3]) * _DAT_004cf3f0;
      fVar7 = _DAT_004cf3f0;
    }
    else {
      local_24 = &DAT_004e5a54;
      local_78 = _DAT_004d5d90 * (local_88 - local_9c[1]);
      local_74 = _DAT_004d5d90 * (local_84 - local_9c[2]);
      local_70 = _DAT_004d5d90 * (local_80 - local_9c[3]);
      local_6c = local_20 * _DAT_004d5d08 + local_9c[0];
      local_68 = (local_88 - local_9c[1]) * _DAT_004d5d08 + local_9c[1];
      local_64 = (local_84 - local_9c[2]) * _DAT_004d5d08 + local_9c[2];
      local_60 = (local_80 - local_9c[3]) * _DAT_004d5d08 + local_9c[3];
      fVar7 = _DAT_004d5d90;
    }
    local_70 = local_70 + local_9c[3];
    local_74 = local_74 + local_9c[2];
    local_78 = local_78 + local_9c[1];
    local_7c = fVar7 * local_20 + local_9c[0];
    param_2 = local_84 - local_9c[2];
    fVar7 = local_88 - local_9c[1];
    local_8 = (float)((int)local_10 + -1);
    if ((int)local_10 + -1 < 0) {
      local_8 = local_8 + _DAT_004cf558;
    }
    if ((short)local_28 == (short)local_2c) {
      fVar1 = 0.0;
    }
    else {
      fVar1 = local_8 / (local_20 * local_20 + fVar7 * fVar7 + param_2 * param_2);
    }
    local_20 = local_20 * fVar1;
    pfVar4 = &local_1d4;
    for (iVar11 = 0x40; iVar11 != 0; iVar11 = iVar11 + -1) {
      *pfVar4 = 0.0;
      pfVar4 = pfVar4 + 1;
    }
    local_1c = fVar1 * fVar7;
    local_18 = fVar1 * param_2;
    SSE2_SetFPControlWord(0);
    local_c = &local_1d4;
    local_2c = local_1cc + (-4 - in_EAX);
    local_58 = local_1c0 + -in_EAX;
    local_54 = local_1a0 + -in_EAX;
    iVar11 = (int)local_2d4 + (8 - in_EAX);
    pfVar4 = (float *)(in_EAX + 4);
    local_50 = local_180 + -in_EAX;
    fVar7 = extraout_EDX_00;
    param_2 = extraout_EDX_00;
    do {
      if ((local_10 != 4.2039e-45) || (_DAT_004cf3f0 <= pfVar4[2])) {
        local_4c = DAT_004fab40 * pfVar4[-1] + *local_c;
        local_48 = DAT_004fab44 * *pfVar4 + *(float *)((int)pfVar4 + local_a8);
        local_44 = DAT_004fab48 * pfVar4[1] + *(float *)(local_2c + (int)pfVar4);
        fVar1 = (local_48 - local_9c[1]) * local_1c +
                (local_44 - local_9c[2]) * local_18 + (local_4c - local_9c[0]) * local_20;
        if (fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) {
          if (fVar1 < local_8) {
            local_28 = (undefined1 *)(fVar1 + _DAT_004cf3f0);
            local_5c = (undefined1 *)(int)ROUND(fVar1 + _DAT_004cf3f0);
            iVar9 = *(int *)(local_24 + (int)local_5c * 4);
          }
          else {
            iVar9 = 1;
          }
        }
        else {
          iVar9 = 0;
        }
        fVar7 = (float)((uint)fVar7 >> 2 | iVar9 << 0x1e);
        fVar2 = (local_4c - local_9c[iVar9 * 4]) * *(float *)(iVar11 + (int)pfVar4);
        uVar5 = (uint)param_2 & 3;
        fVar3 = (local_48 - local_9c[iVar9 * 4 + 1]) * *(float *)(iVar11 + (int)pfVar4);
        local_38 = fVar3;
        fVar1 = (local_44 - local_9c[iVar9 * 4 + 2]) * *(float *)(iVar11 + (int)pfVar4);
        local_34 = fVar1;
        if (uVar5 != 3) {
          *(float *)(local_b8 + (int)pfVar4) =
               fVar2 * _DAT_004e5a78 + *(float *)(local_b8 + (int)pfVar4);
          *(float *)(local_c0 + (int)pfVar4) =
               _DAT_004e5a78 * fVar3 + *(float *)(local_c0 + (int)pfVar4);
          *(float *)(local_58 + (int)pfVar4) =
               fVar1 * _DAT_004e5a78 + *(float *)(local_58 + (int)pfVar4);
        }
        if ((uint)param_2 < 0xc) {
          if (uVar5 != 0) {
            *(float *)(local_b4 + (int)pfVar4) =
                 fVar2 * _DAT_004e5a74 + *(float *)(local_b4 + (int)pfVar4);
            *(float *)(local_ac + (int)pfVar4) =
                 _DAT_004e5a74 * fVar3 + *(float *)(local_ac + (int)pfVar4);
            *(float *)(local_54 + (int)pfVar4) =
                 fVar1 * _DAT_004e5a74 + *(float *)(local_54 + (int)pfVar4);
          }
          *(float *)(local_bc + (int)pfVar4) =
               fVar2 * _DAT_004e5a70 + *(float *)(local_bc + (int)pfVar4);
          *(float *)(local_a4 + (int)pfVar4) =
               fVar3 * _DAT_004e5a70 + *(float *)(local_a4 + (int)pfVar4);
          *(float *)(((int)&local_190 - in_EAX) + (int)pfVar4) =
               fVar1 * _DAT_004e5a70 + *(float *)(((int)&local_190 - in_EAX) + (int)pfVar4);
          if (uVar5 != 3) {
            *(float *)(local_a0 + (int)pfVar4) =
                 fVar2 * _DAT_004db564 + *(float *)(local_a0 + (int)pfVar4);
            *(float *)(local_b0 + (int)pfVar4) =
                 local_38 * _DAT_004db564 + *(float *)(local_b0 + (int)pfVar4);
            *(float *)(local_50 + (int)pfVar4) =
                 local_34 * _DAT_004db564 + *(float *)(local_50 + (int)pfVar4);
          }
        }
      }
      else {
        fVar7 = (float)((uint)fVar7 >> 2 | 0xc0000000);
      }
      param_2 = (float)((int)param_2 + 1);
      local_c = local_c + 4;
      pfVar4 = pfVar4 + 4;
    } while ((uint)param_2 < 0x10);
    *(float *)(param_1 + 2) = fVar7;
  }
  return 0;
}

