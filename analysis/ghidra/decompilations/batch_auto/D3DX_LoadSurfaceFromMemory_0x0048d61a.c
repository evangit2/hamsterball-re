
undefined4 D3DX_LoadSurfaceFromMemory(undefined4 param_1,undefined4 param_2)

{
  uint *puVar1;
  int iVar2;
  void *pvVar3;
  undefined1 *puVar4;
  uint uVar5;
  undefined4 *puVar6;
  undefined4 uVar7;
  undefined4 *puVar8;
  uint *puVar9;
  undefined4 unaff_EDI;
  undefined4 *puVar10;
  code **ppcVar11;
  code *local_278 [2];
  code *local_270;
  undefined4 local_1f4 [16];
  int local_1b4;
  undefined4 *local_1b0;
  undefined4 *local_1a0;
  undefined4 local_170;
  uint local_158;
  uint local_154;
  int local_14c;
  uint local_13c;
  int *local_c;
  undefined4 *local_8;
  
  ppcVar11 = local_278;
  local_1b4 = EH_MediaParser_Record(ppcVar11);
  local_278[0] = Exception_ThrowAndLongjmp;
  local_270 = CRT_Noop2;
  iVar2 = __setjmp3(local_1f4,0,ppcVar11,unaff_EDI);
  if (iVar2 == 0) {
    Sound_Init(&local_1b4,0x3d,0x1a8);
    local_1a0 = (undefined4 *)(*(code *)*local_1b0)(&local_1b4,0,0x24);
    local_1a0[8] = param_2;
    local_1a0[2] = CRT_Noop2;
    local_1a0[3] = &LAB_0048d5f4;
    local_1a0[4] = &LAB_0048d609;
    local_1a0[5] = &LAB_004a4215;
    local_1a0[6] = CRT_Noop2;
    local_1a0[1] = 0;
    *local_1a0 = 0;
    local_1a0[7] = param_1;
    Sound_TickWithNotify(&local_1b4,'\x01');
    iVar2 = CPUID_IsMMXAvailable();
    if (iVar2 == 0) {
      local_170 = 1;
    }
    local_270 = (code *)&LAB_0048d5d7;
    Sound_PlayThrough(&local_1b4);
    if (local_14c == 1) {
      *local_c = 0x32;
      local_c[0xc] = local_158;
    }
    else {
      if (local_14c != 3) goto LAB_0048d665;
      *local_c = 0x16;
      local_c[0xc] = local_158 << 2;
    }
    local_c[0xd] = 0;
    local_c[3] = local_158;
    local_c[4] = local_154;
    local_c[5] = 1;
    if (local_c[0x10] != 0) {
      local_c[0xe] = 1;
      pvVar3 = operator_new(local_c[0xc] * local_154);
      local_c[1] = (int)pvVar3;
      if ((pvVar3 == (void *)0x0) ||
         (local_8 = (undefined4 *)(*(code *)local_1b0[2])(&local_1b4,1,local_158 * local_14c,1),
         local_8 == (undefined4 *)0x0)) {
        uVar7 = 0x8007000e;
        goto LAB_0048d874;
      }
      if (*local_c == 0x32) {
        puVar6 = (undefined4 *)local_c[1];
        if (local_13c < local_154) {
          do {
            Sound_ReadBuffer(&local_1b4,local_8,1);
            puVar8 = (undefined4 *)*local_8;
            puVar10 = puVar6;
            for (uVar5 = local_158 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
              *puVar10 = *puVar8;
              puVar8 = puVar8 + 1;
              puVar10 = puVar10 + 1;
            }
            for (uVar5 = local_158 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
              *(undefined1 *)puVar10 = *(undefined1 *)puVar8;
              puVar8 = (undefined4 *)((int)puVar8 + 1);
              puVar10 = (undefined4 *)((int)puVar10 + 1);
            }
            puVar6 = (undefined4 *)((int)puVar6 + local_158);
          } while (local_13c < local_154);
        }
      }
      else if (*local_c == 0x16) {
        puVar9 = (uint *)local_c[1];
        while (local_13c < local_154) {
          Sound_ReadBuffer(&local_1b4,local_8,1);
          puVar4 = (undefined1 *)*local_8;
          puVar1 = puVar9 + local_158;
          for (; puVar9 < puVar1; puVar9 = puVar9 + 1) {
            *puVar9 = (uint)CONCAT21(CONCAT11(*puVar4,puVar4[1]),puVar4[2]);
            puVar4 = puVar4 + 3;
          }
        }
      }
      Sound_ProcessPlayback(&local_1b4);
    }
    uVar7 = 0;
  }
  else {
LAB_0048d665:
    uVar7 = 0x80004005;
  }
LAB_0048d874:
  Audio_CodecReleaseHandle((int)&local_1b4);
  return uVar7;
}

