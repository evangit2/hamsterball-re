
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ProcessVertices_ZToByte(void *this,uint param_1,uint param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  undefined1 *extraout_EDX;
  undefined1 *puVar3;
  int iVar4;
  
  iVar4 = 0;
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar1 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar1);
  uVar2 = 0;
  puVar3 = extraout_EDX;
  if (*(int *)((int)this + 0x1058) != 0) {
    do {
      *puVar3 = (char)(int)ROUND(*(float *)(iVar4 + 0xc + param_3) * _DAT_004cf304 +
                                 *(float *)(iVar1 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 +
                                           (uVar2 & 3) * 4));
      puVar3 = puVar3 + 1;
      uVar2 = uVar2 + 1;
      iVar4 = iVar4 + 0x10;
    } while (uVar2 < *(uint *)((int)this + 0x1058));
  }
  return;
}

