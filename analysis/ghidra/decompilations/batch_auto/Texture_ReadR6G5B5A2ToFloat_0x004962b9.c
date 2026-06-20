
/* WARNING: Removing unreachable block (ram,0x00496353) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Texture_ReadR6G5B5A2ToFloat(void *this,int param_1,int param_2,float *param_3)

{
  uint *puVar1;
  uint uVar2;
  float fVar3;
  uint *puVar4;
  
  fVar3 = _DAT_004dc0a4;
  puVar4 = (uint *)(*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2
                   + *(int *)((int)this + 0x18));
  puVar1 = puVar4 + *(int *)((int)this + 0x1058);
  for (; puVar4 < puVar1; puVar4 = puVar4 + 1) {
    uVar2 = *puVar4;
    *param_3 = (float)(int)((short)((short)*puVar4 << 6) >> 6) * fVar3;
    param_3[1] = (float)(int)((short)((short)(uVar2 >> 10) << 6) >> 6) * fVar3;
    param_3[2] = (float)(int)((short)((ushort)(uVar2 >> 0x14) << 6) >> 6) * fVar3;
    param_3[3] = (float)(*puVar4 >> 0x1e) * _DAT_004d5d90;
    param_3 = param_3 + 4;
  }
  if (*(int *)((int)this + 0x10) != 0) {
    Mesh_ClearColorVertices(this,(uint)(param_3 + *(int *)((int)this + 0x1058) * -4));
  }
  return;
}

