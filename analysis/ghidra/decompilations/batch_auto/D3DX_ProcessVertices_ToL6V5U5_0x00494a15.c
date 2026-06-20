
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ToL6V5U5(void *this,uint param_1,uint param_2,int param_3)

{
  float fVar1;
  int iVar2;
  ushort *extraout_EDX;
  ushort *puVar3;
  uint uVar4;
  int iVar5;
  
  uVar4 = 0;
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar2 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar2);
  if (*(int *)((int)this + 0x1058) != 0) {
    iVar5 = 0;
    puVar3 = extraout_EDX;
    do {
      fVar1 = *(float *)(iVar2 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (uVar4 & 3) * 4);
      *puVar3 = (ushort)(((int)ROUND(*(float *)(iVar5 + 4 + param_3) * _DAT_004dc07c + fVar1) &
                          0x1fU | (int)ROUND(*(float *)(iVar5 + 0xc + param_3) * _DAT_004dc064 +
                                             fVar1) << 5) << 5) |
                (ushort)(int)ROUND(*(float *)(iVar5 + param_3) * _DAT_004dc07c + fVar1) & 0x1f;
      puVar3 = puVar3 + 1;
      uVar4 = uVar4 + 1;
      iVar5 = iVar5 + 0x10;
    } while (uVar4 < *(uint *)((int)this + 0x1058));
  }
  return;
}

