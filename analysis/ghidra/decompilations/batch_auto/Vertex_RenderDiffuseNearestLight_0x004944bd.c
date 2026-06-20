
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
Vertex_RenderDiffuseNearestLight(void *this,uint param_1,uint param_2,float *param_3)

{
  int iVar1;
  int iVar2;
  float *pfVar3;
  ushort *extraout_EDX;
  ushort *puVar4;
  uint uVar5;
  uint local_14;
  int local_10;
  uint local_c;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = (float *)Vertex_Transform(this,(int)param_3);
  }
  iVar2 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar2);
  uVar5 = 0;
  if (*(int *)((int)this + 0x1058) != 0) {
    local_10 = 0;
    puVar4 = extraout_EDX;
    do {
      local_14 = 0;
      local_c = 0;
      pfVar3 = (float *)((int)this + 0x38);
      do {
        if ((*param_3 - pfVar3[-2]) * (*param_3 - pfVar3[-2]) +
            (param_3[1] - pfVar3[-1]) * (param_3[1] - pfVar3[-1]) +
            (param_3[2] - *pfVar3) * (param_3[2] - *pfVar3) < _DAT_004dc074) {
          local_14 = local_c;
        }
        local_c = local_c + 1;
        pfVar3 = pfVar3 + 4;
      } while (local_c < 0x100);
      iVar1 = local_10 + 0xc;
      local_10 = local_10 + 0x10;
      *puVar4 = (ushort)(byte)(int)ROUND(*(float *)(iVar1 + (int)param_3) * _DAT_004cf304 +
                                         *(float *)(iVar2 + ((param_2 & 3) + (param_1 & 3) * 8) * 4
                                                   + (uVar5 & 3) * 4)) << 8 | (ushort)local_14;
      puVar4 = puVar4 + 1;
      uVar5 = uVar5 + 1;
    } while (uVar5 < *(uint *)((int)this + 0x1058));
  }
  return;
}

