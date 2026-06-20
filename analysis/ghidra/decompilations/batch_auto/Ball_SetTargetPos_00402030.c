/*
 * Function: Ball_SetTargetPos
 * Address: 0x00402030
 * Signature: Ball_SetTargetPos(...)
 *
 * Patterns: ball. Calls: Ball_SetTargetPos, SQRT, Vec3_NormalizeAndScale. Offsets: 4, Lines: 47
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Ball_SetTargetPos(this, x, y, z): Network position sync. If first call: store at
   this+0x76C/770/774, set flag+0x76A=1. Otherwise: lerp toward target by distance*0.1, or snap if
   >1.0. See decomp_ball_input_audio.c */

void __thiscall Ball_SetTargetPos(void *this,float param_1,float param_2,float param_3)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  float local_c;
  float local_8;
  float local_4;
  
  if (*(char *)((int)this + 0x76a) == '\0') {
    if ((float *)((int)this + 0x76c) != &param_1) {
      *(float *)((int)this + 0x76c) = param_1;
      *(float *)((int)this + 0x770) = param_2;
      *(float *)((int)this + 0x774) = param_3;
    }
    *(undefined1 *)((int)this + 0x76a) = 1;
    return;
  }
  local_c = param_1 - *(float *)((int)this + 0x76c);
  pfVar1 = (float *)((int)this + 0x76c);
  local_8 = param_2 - *(float *)((int)this + 0x770);
  local_4 = param_3 - *(float *)((int)this + 0x774);
  fVar2 = local_c * local_c + local_8 * local_8 + local_4 * local_4;
  if ((fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) &&
     (SQRT(fVar2) < _DAT_004cf310 == (SQRT(fVar2) == _DAT_004cf310))) {
    fVar3 = _DAT_004cf368;
    if (fVar2 < _DAT_004cf368 == (fVar2 == _DAT_004cf368)) {
      fVar3 = SQRT(fVar2);
    }
    Vec3_NormalizeAndScale(&local_c,fVar3 * _DAT_004cf3d0);
    *pfVar1 = local_c + *pfVar1;
    *(float *)((int)this + 0x770) = local_8 + *(float *)((int)this + 0x770);
    *(float *)((int)this + 0x774) = local_4 + *(float *)((int)this + 0x774);
  }
  else if (pfVar1 != &param_1) {
    *pfVar1 = param_1;
    *(float *)((int)this + 0x770) = param_2;
    *(float *)((int)this + 0x774) = param_3;
    return;
  }
  return;
}
