
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Texture_ReadA8L8ToFloat(void *this,int param_1,int param_2,float *param_3)

{
  byte *pbVar1;
  float fVar2;
  float fVar3;
  byte *pbVar4;
  
  fVar3 = _DAT_004db358;
  pbVar4 = (byte *)(*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2
                   + *(int *)((int)this + 0x18));
  pbVar1 = pbVar4 + *(int *)((int)this + 0x1058) * 2;
  for (; pbVar4 < pbVar1; pbVar4 = pbVar4 + 2) {
    fVar2 = (float)*pbVar4 * fVar3;
    param_3[2] = fVar2;
    param_3[1] = fVar2;
    *param_3 = fVar2;
    param_3[3] = (float)pbVar4[1] * fVar3;
    param_3 = param_3 + 4;
  }
  if (*(int *)((int)this + 0x10) != 0) {
    Mesh_ClearColorVertices(this,(uint)(param_3 + *(int *)((int)this + 0x1058) * -4));
  }
  return;
}

