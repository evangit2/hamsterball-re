
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Texture_ReadR3G3B2ToFloat(void *this,int param_1,int param_2,float *param_3)

{
  float fVar1;
  byte *pbVar2;
  byte *pbVar3;
  
  fVar1 = _DAT_004d0630;
  pbVar2 = (byte *)(*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2
                   + *(int *)((int)this + 0x18));
  pbVar3 = pbVar2 + *(int *)((int)this + 0x1058);
  for (; pbVar2 < pbVar3; pbVar2 = pbVar2 + 1) {
    *param_3 = (float)(*pbVar2 >> 5) * fVar1;
    param_3[1] = (float)(*pbVar2 >> 2 & 7) * fVar1;
    param_3[2] = (float)(*pbVar2 & 3) * _DAT_004d5d90;
    param_3[3] = 1.0;
    param_3 = param_3 + 4;
  }
  if (*(int *)((int)this + 0x10) != 0) {
    Mesh_ClearColorVertices(this,(uint)(param_3 + *(int *)((int)this + 0x1058) * -4));
  }
  return;
}

