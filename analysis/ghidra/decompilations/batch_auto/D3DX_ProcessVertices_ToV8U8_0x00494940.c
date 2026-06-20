
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ToV8U8(void *this,uint param_1,uint param_2,int param_3)

{
  float *pfVar1;
  int iVar2;
  float fVar3;
  int iVar4;
  uint uVar5;
  undefined2 *extraout_EDX;
  undefined2 *puVar6;
  int iVar7;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar4 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar4);
  uVar5 = 0;
  if (*(int *)((int)this + 0x1058) != 0) {
    iVar7 = 0;
    puVar6 = extraout_EDX;
    do {
      fVar3 = *(float *)(iVar4 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (uVar5 & 3) * 4);
      pfVar1 = (float *)(iVar7 + param_3);
      iVar2 = iVar7 + 4;
      iVar7 = iVar7 + 0x10;
      *puVar6 = CONCAT11((char)(int)ROUND(*(float *)(iVar2 + param_3) * _DAT_004dc078 + fVar3),
                         (char)(int)ROUND(*pfVar1 * _DAT_004dc078 + fVar3));
      puVar6 = puVar6 + 1;
      uVar5 = uVar5 + 1;
    } while (uVar5 < *(uint *)((int)this + 0x1058));
  }
  return;
}

