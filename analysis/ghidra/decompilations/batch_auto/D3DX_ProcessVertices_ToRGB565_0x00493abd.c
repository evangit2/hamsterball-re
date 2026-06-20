
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ToRGB565(void *this,uint param_1,uint param_2,int param_3)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  ushort *extraout_EDX;
  ushort *puVar7;
  uint uVar8;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar5 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar5);
  uVar8 = 0;
  if (*(int *)((int)this + 0x1058) != 0) {
    iVar6 = 0;
    puVar7 = extraout_EDX;
    do {
      fVar4 = *(float *)(iVar5 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (uVar8 & 3) * 4);
      pfVar1 = (float *)(iVar6 + param_3);
      iVar2 = iVar6 + 4;
      iVar3 = iVar6 + 8;
      iVar6 = iVar6 + 0x10;
      *puVar7 = (ushort)(((int)ROUND(*pfVar1 * _DAT_004d5c24 + fVar4) << 6 |
                         (int)ROUND(*(float *)(iVar2 + param_3) * _DAT_004dc064 + fVar4)) << 5) |
                (ushort)(int)ROUND(*(float *)(iVar3 + param_3) * _DAT_004d5c24 + fVar4);
      puVar7 = puVar7 + 1;
      uVar8 = uVar8 + 1;
    } while (uVar8 < *(uint *)((int)this + 0x1058));
  }
  return;
}

