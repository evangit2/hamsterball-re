
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ToDec3(void *this,uint param_1,uint param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  float fVar4;
  int iVar5;
  float fVar6;
  int iVar7;
  uint *puVar8;
  uint local_c;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  puVar8 = (uint *)(*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2
                   + *(int *)((int)this + 0x18));
  iVar5 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar5);
  local_c = 0;
  if (*(int *)((int)this + 0x1058) != 0) {
    iVar7 = 0;
    do {
      fVar4 = *(float *)(iVar5 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (local_c & 3) * 4);
      uVar1 = (uint)ROUND(*(float *)(iVar7 + param_3) * _DAT_004dc070 + fVar4);
      uVar2 = (uint)ROUND(*(float *)(iVar7 + 4 + param_3) * _DAT_004dc070 + fVar4);
      uVar3 = (uint)ROUND(*(float *)(iVar7 + 8 + param_3) * _DAT_004dc070 + fVar4);
      fVar6 = *(float *)(iVar7 + 0xc + param_3) * _DAT_004dc070;
      *puVar8 = uVar2 << 0x10 | uVar3;
      puVar8[1] = (((int)ROUND(fVar6 + fVar4) << 0x10 | uVar1) >> 0x10 | (int)uVar2 >> 0x1f) << 0x10
                  | (uVar1 << 0x10 | uVar2) >> 0x10 | (int)uVar3 >> 0x1f;
      puVar8 = puVar8 + 2;
      local_c = local_c + 1;
      iVar7 = iVar7 + 0x10;
    } while (local_c < *(uint *)((int)this + 0x1058));
  }
  return;
}

