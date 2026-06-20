
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ExtractTexCoords_FromA8P8(void *this,int param_1,int param_2,float *param_3)

{
  byte *pbVar1;
  float fVar2;
  byte *pbVar3;
  
  fVar2 = _DAT_004d0630;
  pbVar3 = (byte *)(*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2
                   + *(int *)((int)this + 0x18));
  pbVar1 = pbVar3 + *(int *)((int)this + 0x1058) * 2;
  for (; pbVar3 < pbVar1; pbVar3 = pbVar3 + 2) {
    *param_3 = (float)(*pbVar3 >> 5) * fVar2;
    param_3[1] = (float)(*pbVar3 >> 2 & 7) * fVar2;
    param_3[2] = (float)(*pbVar3 & 3) * _DAT_004d5d90;
    param_3[3] = (float)pbVar3[1] * _DAT_004db358;
    param_3 = param_3 + 4;
  }
  if (*(int *)((int)this + 0x10) != 0) {
    Mesh_ClearColorVertices(this,(uint)(param_3 + *(int *)((int)this + 0x1058) * -4));
  }
  return;
}

