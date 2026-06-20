
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Vertex_RenderSpecularARGB(void *this,uint param_1,uint param_2,int param_3)

{
  float fVar1;
  int iVar2;
  int iVar3;
  uint *extraout_EDX;
  uint *puVar4;
  uint local_c;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar2 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar2);
  local_c = 0;
  if (*(int *)((int)this + 0x1058) != 0) {
    iVar3 = 0;
    puVar4 = extraout_EDX;
    do {
      fVar1 = *(float *)(iVar2 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (local_c & 3) * 4);
      *puVar4 = (((int)ROUND(*(float *)(iVar3 + 8 + param_3) * _DAT_004dc078 + fVar1) & 0xffU |
                 (int)ROUND(*(float *)(iVar3 + 0xc + param_3) * _DAT_004dc078 + fVar1) << 8) << 8 |
                (int)ROUND(*(float *)(iVar3 + 4 + param_3) * _DAT_004dc078 + fVar1) & 0xffU) << 8 |
                (int)ROUND(*(float *)(iVar3 + param_3) * _DAT_004dc078 + fVar1) & 0xffU;
      puVar4 = puVar4 + 1;
      local_c = local_c + 1;
      iVar3 = iVar3 + 0x10;
    } while (local_c < *(uint *)((int)this + 0x1058));
  }
  return;
}

