
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ToA2R10G10B10(void *this,uint param_1,uint param_2,int param_3)

{
  float fVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  uint *extraout_EDX;
  uint *puVar5;
  uint local_c;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar3 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar3);
  local_c = 0;
  if (*(int *)((int)this + 0x1058) != 0) {
    iVar4 = 0;
    puVar5 = extraout_EDX;
    do {
      fVar1 = *(float *)(iVar3 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (local_c & 3) * 4);
      fVar2 = *(float *)(iVar4 + 0xc + param_3);
      *puVar5 = (((int)ROUND(*(float *)(iVar4 + 8 + param_3) * _DAT_004dc088 + fVar1) & 0x3ffU |
                 (int)ROUND(fVar2 + fVar2 + fVar1) << 10) << 10 |
                (int)ROUND(*(float *)(iVar4 + 4 + param_3) * _DAT_004dc088 + fVar1) & 0x3ffU) << 10
                | (int)ROUND(*(float *)(iVar4 + param_3) * _DAT_004dc088 + fVar1) & 0x3ffU;
      puVar5 = puVar5 + 1;
      local_c = local_c + 1;
      iVar4 = iVar4 + 0x10;
    } while (local_c < *(uint *)((int)this + 0x1058));
  }
  return;
}

