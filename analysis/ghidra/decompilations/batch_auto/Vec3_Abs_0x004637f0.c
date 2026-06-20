
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Vec3_Abs(void *this,float *param_1)

{
  float fVar1;
  float local_c;
  float local_8;
  
  if (_DAT_004cf368 <= *(float *)this) {
    local_c = *(float *)this;
  }
  else {
    local_c = -*(float *)this;
  }
  if (_DAT_004cf368 <= *(float *)((int)this + 4)) {
    local_8 = *(float *)((int)this + 4);
  }
  else {
    local_8 = -*(float *)((int)this + 4);
  }
  fVar1 = *(float *)((int)this + 8);
  if (*(float *)((int)this + 8) < _DAT_004cf368) {
    fVar1 = -fVar1;
  }
  if (param_1 != &local_c) {
    param_1[2] = fVar1;
    *param_1 = local_c;
    param_1[1] = local_8;
    return;
  }
  return;
}

