
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ExtractTexCoords_FromA4L4_4(void *this,int param_1,int param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  byte *pbVar3;
  byte *pbVar4;
  
  fVar2 = _DAT_004dc090;
  pbVar3 = (byte *)(*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2
                   + *(int *)((int)this + 0x18));
  pbVar4 = pbVar3 + *(int *)((int)this + 0x1058);
  for (; pbVar3 < pbVar4; pbVar3 = pbVar3 + 1) {
    fVar1 = (float)(*pbVar3 & 0xf) * fVar2;
    param_3[2] = fVar1;
    param_3[1] = fVar1;
    *param_3 = fVar1;
    param_3[3] = (float)(*pbVar3 >> 4) * fVar2;
    param_3 = param_3 + 4;
  }
  if (*(int *)((int)this + 0x10) != 0) {
    Mesh_ClearColorVertices(this,(uint)(param_3 + *(int *)((int)this + 0x1058) * -4));
  }
  return;
}

