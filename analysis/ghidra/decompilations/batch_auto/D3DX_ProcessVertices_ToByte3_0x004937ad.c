
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ToByte3(void *this,uint param_1,uint param_2,int param_3)

{
  float fVar1;
  int iVar2;
  int iVar3;
  undefined1 *extraout_EDX;
  undefined1 *puVar4;
  uint uVar5;
  
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar2 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar2);
  iVar3 = 0;
  uVar5 = 0;
  puVar4 = extraout_EDX;
  if (*(int *)((int)this + 0x1058) != 0) {
    do {
      fVar1 = *(float *)(iVar2 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 + (uVar5 & 3) * 4);
      *puVar4 = (char)(int)ROUND(*(float *)(iVar3 + 8 + param_3) * _DAT_004cf304 + fVar1);
      puVar4[1] = (char)(int)ROUND(*(float *)(iVar3 + 4 + param_3) * _DAT_004cf304 + fVar1);
      puVar4[2] = (char)(int)ROUND(*(float *)(iVar3 + param_3) * _DAT_004cf304 + fVar1);
      puVar4 = puVar4 + 3;
      uVar5 = uVar5 + 1;
      iVar3 = iVar3 + 0x10;
    } while (uVar5 < *(uint *)((int)this + 0x1058));
  }
  return;
}

