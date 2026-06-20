
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ToX8L8V8U8(void *this,uint param_1,uint param_2,int param_3)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  float fVar4;
  int iVar5;
  uint uVar6;
  uint *extraout_EDX;
  uint *puVar7;
  int iVar8;
  undefined1 local_24;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar5 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar5);
  uVar6 = 0;
  if (*(int *)((int)this + 0x1058) != 0) {
    iVar8 = 0;
    puVar7 = extraout_EDX;
    do {
      fVar4 = *(float *)(iVar5 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (uVar6 & 3) * 4);
      pfVar1 = (float *)(iVar8 + param_3);
      iVar2 = iVar8 + 4;
      iVar3 = iVar8 + 0xc;
      iVar8 = iVar8 + 0x10;
      local_24 = (undefined1)(int)ROUND(*(float *)(iVar2 + param_3) * _DAT_004dc078 + fVar4);
      *puVar7 = (uint)CONCAT11((char)(int)ROUND(*(float *)(iVar3 + param_3) * _DAT_004cf304 + fVar4)
                               ,local_24) << 8 | (int)ROUND(*pfVar1 * _DAT_004dc078 + fVar4) & 0xffU
      ;
      puVar7 = puVar7 + 1;
      uVar6 = uVar6 + 1;
    } while (uVar6 < *(uint *)((int)this + 0x1058));
  }
  return;
}

