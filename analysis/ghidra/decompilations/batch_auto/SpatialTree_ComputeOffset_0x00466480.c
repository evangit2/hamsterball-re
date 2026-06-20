
void __thiscall
SpatialTree_ComputeOffset(void *this,undefined4 param_1,undefined4 param_2,int param_3,int param_4)

{
  float fVar1;
  float fVar2;
  float *pfVar3;
  float local_c;
  
  pfVar3 = (float *)(float)param_4;
  fVar1 = *(float *)((int)this + 8);
  fVar2 = *(float *)((int)this + 0xc);
  Graphics_InitShaderDispatch();
  *pfVar3 = fVar1 - (float)param_3;
  pfVar3[1] = fVar2 - (float)pfVar3;
  pfVar3[2] = local_c;
  return;
}

