
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
D3DX_ExtractTexCoords_FromAlpha8(void *this,int param_1,int param_2,undefined4 *param_3)

{
  byte *pbVar1;
  byte *pbVar2;
  
  pbVar1 = (byte *)(*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2
                   + *(int *)((int)this + 0x18));
  pbVar2 = pbVar1 + *(int *)((int)this + 0x1058);
  for (; pbVar1 < pbVar2; pbVar1 = pbVar1 + 1) {
    *param_3 = 0;
    param_3[1] = 0;
    param_3[2] = 0;
    param_3[3] = (float)*pbVar1 * _DAT_004db358;
    param_3 = param_3 + 4;
  }
  if (*(int *)((int)this + 0x10) != 0) {
    Mesh_ClearColorVertices(this,(uint)(param_3 + *(int *)((int)this + 0x1058) * -4));
  }
  return;
}

