
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl IMDCT_TransformBlock(undefined4 *param_1,int param_2)

{
  uint *puVar1;
  float fVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 *extraout_EDX;
  uint uVar5;
  float *pfVar6;
  float local_54 [16];
  int local_14;
  int local_10;
  float local_c;
  float *local_8;
  
  puVar3 = param_1;
  *param_1 = 0;
  param_1[1] = 0;
  pfVar6 = local_54;
  for (iVar4 = 0x10; iVar4 != 0; iVar4 = iVar4 + -1) {
    *pfVar6 = 0.0;
    pfVar6 = pfVar6 + 1;
  }
  SSE2_SetFPControlWord(0);
  local_8 = (float *)(param_2 + 0xc);
  param_1 = extraout_EDX;
  do {
    local_c = local_54[(int)param_1] + *local_8;
    local_14 = (int)ROUND(local_c * _DAT_004d039c + _DAT_004cf3f0);
    puVar1 = puVar3 + ((uint)param_1 >> 3);
    local_10 = local_14;
    fVar2 = (float)local_14;
    *puVar1 = *puVar1 >> 4 | local_14 << 0x1c;
    if (local_14 < 0) {
      fVar2 = fVar2 + _DAT_004cf558;
    }
    uVar5 = (uint)param_1 & 3;
    fVar2 = local_c - fVar2 * _DAT_004dc090;
    if (uVar5 != 3) {
      local_54[(int)param_1 + 1] = _DAT_004e5a78 * fVar2 + local_54[(int)param_1 + 1];
    }
    if (param_1 < (undefined4 *)0xc) {
      if (uVar5 != 0) {
        local_54[(int)param_1 + 3] = _DAT_004e5a74 * fVar2 + local_54[(int)param_1 + 3];
      }
      local_54[(int)(param_1 + 1)] = _DAT_004e5a70 * fVar2 + local_54[(int)(param_1 + 1)];
      if (uVar5 != 3) {
        local_54[(int)param_1 + 5] = fVar2 * _DAT_004db564 + local_54[(int)param_1 + 5];
      }
    }
    param_1 = (undefined4 *)((int)param_1 + 1);
    local_8 = local_8 + 4;
  } while (param_1 < (undefined4 *)0x10);
  DXT_CompressBlock1((ushort *)(puVar3 + 2),0.0);
  return;
}

