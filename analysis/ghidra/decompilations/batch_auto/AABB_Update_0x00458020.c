
void __thiscall AABB_Update(void *this,int param_1,int param_2,int param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  
  fVar1 = (float)param_1;
  if (fVar1 < *(float *)this) {
    *(float *)this = fVar1;
  }
  fVar2 = (float)param_2;
  if (fVar2 < *(float *)((int)this + 4)) {
    *(float *)((int)this + 4) = fVar2;
  }
  fVar3 = (float)param_3;
  if (fVar3 < *(float *)((int)this + 8)) {
    *(float *)((int)this + 8) = fVar3;
  }
  if (*(float *)((int)this + 0xc) < fVar1) {
    *(float *)((int)this + 0xc) = fVar1;
  }
  if (*(float *)((int)this + 0x10) < fVar2) {
    *(float *)((int)this + 0x10) = fVar2;
  }
  if (*(float *)((int)this + 0x14) < fVar3) {
    *(float *)((int)this + 0x14) = fVar3;
    return;
  }
  return;
}

