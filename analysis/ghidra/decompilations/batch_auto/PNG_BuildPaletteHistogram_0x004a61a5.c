
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl PNG_BuildPaletteHistogram(int *param_1)

{
  ushort *puVar1;
  int *piVar2;
  byte bVar3;
  int *piVar4;
  void *pvVar5;
  uint uVar6;
  byte bVar7;
  int iVar8;
  uint uVar9;
  int iVar10;
  uint uVar11;
  longlong lVar12;
  ushort local_10;
  uint local_8;
  
  piVar4 = param_1;
  if ((float)param_1[0x4c] != (float)_DAT_004cf6a8) {
    if (*(byte *)((int)param_1 + 0x117) < 9) {
      pvVar5 = Malloc_OrLongjmp(param_1,0x100);
      param_1 = (int *)0x0;
      piVar4[0x4e] = (int)pvVar5;
      do {
        FPU_FdivThunk();
        lVar12 = __ftol();
        piVar2 = (int *)((int)param_1 + 1);
        *(char *)((int)param_1 + piVar4[0x4e]) = (char)lVar12;
        param_1 = piVar2;
      } while ((int)piVar2 < 0x100);
    }
    else {
      if ((*(byte *)((int)param_1 + 0x116) & 2) == 0) {
        local_8 = (uint)*(byte *)((int)param_1 + 0x153);
      }
      else {
        local_8 = (uint)*(byte *)(param_1 + 0x54);
        if ((uint)*(byte *)(param_1 + 0x54) < (uint)*(byte *)((int)param_1 + 0x151)) {
          local_8 = (uint)*(byte *)((int)param_1 + 0x151);
        }
        if (local_8 < *(byte *)((int)param_1 + 0x152)) {
          local_8 = (uint)*(byte *)((int)param_1 + 0x152);
        }
      }
      if (local_8 == 0) {
        local_8 = 0;
      }
      else {
        local_8 = 0x10 - local_8;
      }
      if (((*(byte *)((int)param_1 + 0x61) & 4) != 0) && ((int)local_8 < 5)) {
        local_8 = 5;
      }
      if (8 < (int)local_8) {
        local_8 = 8;
      }
      if ((int)local_8 < 0) {
        local_8 = 0;
      }
      bVar3 = (byte)local_8;
      bVar7 = 8 - bVar3;
      iVar8 = 1 << (bVar7 & 0x1f);
      param_1[0x4b] = local_8 & 0xff;
      pvVar5 = Malloc_OrLongjmp(param_1,iVar8 << 2);
      puVar1 = (ushort *)(param_1 + 0x18);
      param_1[0x51] = (int)pvVar5;
      param_1 = (int *)0x0;
      if ((*puVar1 & 0x480) == 0) {
        if (0 < iVar8) {
          do {
            pvVar5 = Malloc_OrLongjmp(piVar4,0x200);
            *(void **)(piVar4[0x51] + (int)param_1 * 4) = pvVar5;
            iVar10 = 0;
            do {
              FPU_FdivThunk();
              lVar12 = __ftol();
              *(short *)(iVar10 + *(int *)(piVar4[0x51] + (int)param_1 * 4)) = (short)lVar12;
              iVar10 = iVar10 + 2;
            } while (iVar10 < 0x200);
            param_1 = (int *)((int)param_1 + 1);
          } while ((int)param_1 < iVar8);
        }
      }
      else {
        if (0 < iVar8) {
          do {
            pvVar5 = Malloc_OrLongjmp(piVar4,0x200);
            piVar2 = (int *)((int)param_1 + 1);
            *(void **)(piVar4[0x51] + (int)param_1 * 4) = pvVar5;
            param_1 = piVar2;
          } while ((int)piVar2 < iVar8);
        }
        uVar11 = 0;
        param_1 = (int *)0x0;
        do {
          FPU_FdivThunk();
          lVar12 = __ftol();
          if (uVar11 <= (uint)lVar12) {
            local_10 = (ushort)(((uint)param_1 & 0xff) << 8) | (ushort)param_1;
            do {
              uVar9 = uVar11 >> (bVar7 & 0x1f);
              uVar6 = 0xff >> (bVar3 & 0x1f) & uVar11;
              uVar11 = uVar11 + 1;
              *(ushort *)(*(int *)(piVar4[0x51] + uVar6 * 4) + uVar9 * 2) = local_10;
            } while (uVar11 <= (uint)lVar12);
          }
          param_1 = (int *)((int)param_1 + 1);
        } while ((int)param_1 < 0x100);
        if (uVar11 < (uint)(iVar8 << 8)) {
          do {
            *(undefined2 *)
             (*(int *)(piVar4[0x51] + (0xff >> (bVar3 & 0x1f) & uVar11) * 4) +
             (uVar11 >> (bVar7 & 0x1f)) * 2) = 0xffff;
            uVar11 = uVar11 + 1;
          } while (uVar11 < (uint)(iVar8 << 8));
        }
      }
    }
  }
  return;
}

