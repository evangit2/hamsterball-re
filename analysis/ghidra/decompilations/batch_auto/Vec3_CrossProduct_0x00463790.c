
void __thiscall Vec3_CrossProduct(void *this,float *param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float local_c [3];
  
  fVar1 = *(float *)((int)this + 8);
  fVar2 = *param_2;
  fVar3 = *(float *)this;
  fVar4 = param_2[2];
  fVar5 = *(float *)this;
  fVar6 = param_2[1];
  fVar7 = *(float *)((int)this + 4);
  fVar8 = *param_2;
  if (param_1 != local_c) {
    *param_1 = *(float *)((int)this + 4) * param_2[2] - *(float *)((int)this + 8) * param_2[1];
    param_1[1] = fVar1 * fVar2 - fVar3 * fVar4;
    param_1[2] = fVar5 * fVar6 - fVar7 * fVar8;
    return;
  }
  return;
}

