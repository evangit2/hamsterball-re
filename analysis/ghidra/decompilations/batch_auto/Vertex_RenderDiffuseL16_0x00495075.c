
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Vertex_RenderDiffuseL16(void *this,uint param_1,uint param_2,int param_3)

{
  int iVar1;
  float *pfVar2;
  uint uVar3;
  undefined2 *extraout_EDX;
  undefined2 *puVar4;
  int iVar5;
  
  iVar5 = 0;
  if (*(int *)((int)this + 0x1048) != 0) {
    param_3 = Vertex_Transform(this,param_3);
  }
  iVar1 = *(int *)((int)this + 0x2c);
  D3DDevice_SetFPUControl(iVar1);
  uVar3 = 0;
  puVar4 = extraout_EDX;
  if (*(int *)((int)this + 0x1058) != 0) {
    do {
      pfVar2 = (float *)(param_3 + iVar5);
      *puVar4 = (short)(int)ROUND(*(float *)(iVar1 + ((param_2 & 3) + (param_1 & 3) * 8) * 4 +
                                            (uVar3 & 3) * 4) +
                                  (*pfVar2 * _DAT_004d8f7c +
                                  pfVar2[1] * _DAT_004d8f74 + pfVar2[2] * _DAT_004d8f78) *
                                  _DAT_004dc070);
      puVar4 = puVar4 + 1;
      uVar3 = uVar3 + 1;
      iVar5 = iVar5 + 0x10;
    } while (uVar3 < *(uint *)((int)this + 0x1058));
  }
  return;
}

