/*
 * Function: Scene_ComputeLighting
 * Address: 0x0041a9a0
 * Signature: Scene_ComputeLighting(...)
 *
 * Patterns: input, ball, scene. Calls: Scene_ComputeLighting, Ball_GetInputForce, Vec2_Distance, Graphics_SetStreamBuffers, __ftol2, SQRT. Offsets: 6, Lines: 91
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Scene_ComputeLighting(void *this,float *param_1,int param_2)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  int iVar4;
  float *pfVar5;
  int iVar6;
  float10 fVar7;
  float local_28;
  float local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_c;
  float local_8;
  float local_4;
  
  bVar3 = false;
  local_18 = 0.0;
  local_14 = 0.0;
  local_28 = 0.0;
  iVar4 = 3;
  iVar6 = 0;
  if (*(char *)(*(int *)((int)this + 0x878) + 0x234) != '\0') {
    iVar4 = *(int *)(*(int *)((int)this + 0x878) + 0xb28 + param_2 * 4);
    iVar6 = iVar4;
  }
  if (iVar6 <= iVar4) {
    param_2 = (iVar4 - iVar6) + 1;
    iVar4 = iVar6 * 4 + 0x550;
    bVar3 = false;
    do {
      Ball_GetInputForce(*(void **)(iVar4 + *(int *)((int)this + 0x878)),&local_24);
      if (((local_24 != _DAT_004cf368) || (local_20 != _DAT_004cf368)) &&
         (fVar7 = Vec2_Distance(0.0,0.0,local_24,local_20), (float10)local_28 < fVar7)) {
        local_14 = local_20;
        local_18 = local_24;
        fVar7 = Vec2_Distance(0.0,0.0,local_24,local_20);
        local_28 = (float)fVar7;
        bVar3 = false;
        if (*(int *)(*(int *)(iVar4 + *(int *)((int)this + 0x878)) + 8) == 2) {
          bVar3 = true;
        }
      }
      iVar4 = iVar4 + 4;
      param_2 = param_2 + -1;
    } while (param_2 != 0);
  }
  Graphics_SetStreamBuffers(*(int *)(*(int *)((int)this + 0x878) + 0x174));
  if (local_28 == _DAT_004cf368) {
    local_24 = local_c;
    local_20 = local_8;
    local_1c = local_4;
  }
  else {
    iVar4 = *(int *)((int)this + 0x878);
    __ftol2();
    __ftol2();
    pfVar5 = (float *)Graphics_SetStreamBuffers(*(int *)(iVar4 + 0x174));
    if (&local_24 != pfVar5) {
      local_24 = *pfVar5;
      local_20 = pfVar5[1];
      local_1c = pfVar5[2];
    }
  }
  local_24 = local_24 - local_c;
  local_20 = local_20 - local_8;
  local_1c = local_1c - local_4;
  fVar1 = local_24 * local_24 + local_20 * local_20 + local_1c * local_1c;
  fVar2 = _DAT_004cf368;
  if ((fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) &&
     (fVar2 = SQRT(fVar1), _DAT_004cf368 < fVar2)) {
    fVar2 = _DAT_004cf310 / fVar2;
  }
  fVar1 = _DAT_004d03b8;
  if (bVar3) {
    fVar1 = *(float *)(*(int *)((int)this + 0x878) + 0x84c) * (float)_DAT_004d03b0 +
            (float)_DAT_004d03a8;
  }
  if (param_1 == &local_18) {
    return;
  }
  param_1[2] = fVar1 * fVar2 * local_1c;
  *param_1 = local_24 * fVar2 * fVar1;
  param_1[1] = local_20 * fVar2 * fVar1;
  return;
}
