
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ToRGB4444(void *this,uint param_1,uint param_2,int param_3)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  float fVar5;
  int iVar6;
  int iVar7;
  ushort *extraout_EDX;
  ushort *puVar8;
  uint uVar9;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar6 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar6);
  uVar9 = 0;
  if (*(int *)((int)this + 0x1058) != 0) {
    iVar7 = 0;
    puVar8 = extraout_EDX;
    do {
      fVar5 = *(float *)(iVar6 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (uVar9 & 3) * 4);
      pfVar1 = (float *)(iVar7 + param_3);
      iVar2 = iVar7 + 4;
      iVar3 = iVar7 + 8;
      iVar4 = iVar7 + 0xc;
      iVar7 = iVar7 + 0x10;
      *puVar8 = (ushort)((((int)ROUND(*(float *)(iVar4 + param_3) * _DAT_004d039c + fVar5) << 4 |
                          (int)ROUND(*pfVar1 * _DAT_004d039c + fVar5)) << 4 |
                         (int)ROUND(*(float *)(iVar2 + param_3) * _DAT_004d039c + fVar5)) << 4) |
                (ushort)(int)ROUND(*(float *)(iVar3 + param_3) * _DAT_004d039c + fVar5);
      puVar8 = puVar8 + 1;
      uVar9 = uVar9 + 1;
    } while (uVar9 < *(uint *)((int)this + 0x1058));
  }
  return;
}

