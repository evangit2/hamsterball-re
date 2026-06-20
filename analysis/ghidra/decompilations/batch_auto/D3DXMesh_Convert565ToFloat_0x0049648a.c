
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DXMesh_Convert565ToFloat(void *this,int param_1,int param_2,float *param_3)

{
  float fVar1;
  ushort *puVar2;
  ushort *puVar3;
  
  fVar1 = _DAT_004dc098;
  puVar2 = (ushort *)
           (*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2 +
           *(int *)((int)this + 0x18));
  puVar3 = (ushort *)(*(int *)((int)this + 0x1064) + (int)puVar2);
  for (; puVar2 < puVar3; puVar2 = puVar2 + 3) {
    *param_3 = (float)puVar2[2] * fVar1;
    param_3[1] = (float)puVar2[1] * fVar1;
    param_3[2] = (float)*puVar2 * fVar1;
    param_3[3] = 1.0;
    param_3 = param_3 + 4;
  }
  if (*(int *)((int)this + 0x10) != 0) {
    Mesh_ClearColorVertices(this,(uint)(param_3 + *(int *)((int)this + 0x1058) * -4));
  }
  return;
}

