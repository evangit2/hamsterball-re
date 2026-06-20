
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ToAlpha4(void *this,uint param_1,uint param_2,int param_3)

{
  float *pfVar1;
  int iVar2;
  float fVar3;
  int iVar4;
  byte *extraout_EDX;
  byte *pbVar5;
  int iVar6;
  uint uVar7;
  byte local_20;
  
  uVar7 = 0;
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar4 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar4);
  if (*(int *)((int)this + 0x1058) != 0) {
    iVar6 = 0;
    pbVar5 = extraout_EDX;
    do {
      fVar3 = *(float *)(iVar4 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (uVar7 & 3) * 4);
      pfVar1 = (float *)(iVar6 + param_3);
      iVar2 = iVar6 + 8;
      local_20 = (byte)(int)ROUND(*(float *)(iVar6 + 4 + param_3) * _DAT_004dc068 + fVar3);
      iVar6 = iVar6 + 0x10;
      *pbVar5 = ((char)(int)ROUND(*pfVar1 * _DAT_004dc068 + fVar3) << 3 | local_20) << 2 |
                (byte)(int)ROUND(*(float *)(iVar2 + param_3) * _DAT_004cf418 + fVar3);
      pbVar5 = pbVar5 + 1;
      uVar7 = uVar7 + 1;
    } while (uVar7 < *(uint *)((int)this + 0x1058));
  }
  return;
}

