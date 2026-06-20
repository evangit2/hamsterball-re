
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall D3DX_ExtractTexCoords_FromV8U8(void *this,int param_1,int param_2,float *param_3)

{
  char *pcVar1;
  float fVar2;
  char *pcVar3;
  
  fVar2 = _DAT_004dc09c;
  pcVar3 = (char *)(*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2
                   + *(int *)((int)this + 0x18));
  pcVar1 = pcVar3 + *(int *)((int)this + 0x1058) * 2;
  for (; pcVar3 < pcVar1; pcVar3 = pcVar3 + 2) {
    *param_3 = (float)(int)*pcVar3 * fVar2;
    param_3[1] = (float)(int)pcVar3[1] * fVar2;
    param_3[2] = 0.0;
    param_3[3] = 1.0;
    param_3 = param_3 + 4;
  }
  if (*(int *)((int)this + 0x10) != 0) {
    Mesh_ClearColorVertices(this,(uint)(param_3 + *(int *)((int)this + 0x1058) * -4));
  }
  return;
}

