
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
D3DX_ExtractTexCoords_FromP8A8(void *this,int param_1,int param_2,undefined4 *param_3)

{
  byte *pbVar1;
  byte *pbVar2;
  undefined4 *puVar3;
  
  pbVar2 = (byte *)(*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2
                   + *(int *)((int)this + 0x18));
  pbVar1 = pbVar2 + *(int *)((int)this + 0x1058) * 2;
  for (; pbVar2 < pbVar1; pbVar2 = pbVar2 + 2) {
    puVar3 = (undefined4 *)((*pbVar2 + 3) * 0x10 + (int)this);
    *param_3 = *puVar3;
    param_3[1] = puVar3[1];
    param_3[2] = puVar3[2];
    param_3[3] = puVar3[3];
    param_3[3] = (float)pbVar2[1] * _DAT_004db358;
    param_3 = param_3 + 4;
  }
  if (*(int *)((int)this + 0x10) != 0) {
    Mesh_ClearColorVertices(this,(uint)(param_3 + *(int *)((int)this + 0x1058) * -4));
  }
  return;
}

