/*
 * Function: Scene_SetCamera
 * Address: 0x00419fa0
 * Signature: Scene_SetCamera(...)
 *
 * Patterns: vtable dispatch, camera, ball, scene. Calls: rail, Shake, Snap, Orbit, Scene_SetCamera, Path_GetPosition, SQRT, Wave_Sin. Offsets: 22, Lines: 112
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Scene_SetCamera: 5 modes - Default follow, Path rail(spring+sin), Shake(random+/-50),
   Snap(countdown frames), Orbit(sin/cos rotation). Ball cam target at +0x758, actual at +0x76C. See
   docs/SCENE_SYSTEM_DECOMP.md */

void __thiscall Scene_SetCamera(void *this,void *param_1,char param_2)

{
  float *pfVar1;
  int iVar2;
  int iVar3;
  undefined1 auVar4 [10];
  float fVar5;
  float fVar6;
  float local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  float local_4;
  
  pfVar1 = (float *)((int)param_1 + 0x758);
  if (&local_24 != pfVar1) {
    local_24 = *pfVar1;
    local_20 = *(float *)((int)param_1 + 0x75c);
    local_1c = *(float *)((int)param_1 + 0x760);
  }
  local_24 = local_24 + *(float *)((int)this + 0x434c);
  local_20 = local_20 + *(float *)((int)this + 0x4350);
  local_1c = local_1c + *(float *)((int)this + 0x4354);
  if ((*(int *)((int)this + 0x3f1c) != 0) && (param_2 != '\0')) {
    fVar5 = 6.026756e-39;
    Path_GetPosition(*(void **)((int)this + 0x3f20),&local_c,*(float *)((int)this + 0x3f24));
    local_18 = local_24 - local_c;
    local_14 = local_20 - local_8;
    local_10 = local_1c - local_4;
    _param_2 = local_14 * local_14 + local_18 * local_18 + local_10 * local_10;
    if (_param_2 < _DAT_004cf368 == (_param_2 == _DAT_004cf368)) {
      _param_2 = SQRT(_param_2);
      if (_DAT_004cf3ec < _param_2) {
        if (_DAT_004cff78 < _param_2) {
          _param_2 = 700.0;
        }
        _param_2 = _param_2 - _DAT_004cf3ec;
        fVar6 = _param_2 + _DAT_004cf3ec;
        auVar4 = Wave_Sin(&PTR_PTR_004f7188,_param_2 * _DAT_004d03a4 * _DAT_004d03a0);
        _param_2 = (float)((float10)fVar6 -
                          (float10)_param_2 * (float10)_DAT_004cf3f0 * (float10)auVar4);
      }
    }
    else {
      _param_2 = 0.0;
    }
    fVar6 = 6.02706e-39;
    Vec3_NormalizeAndScale(&local_18,_param_2);
    local_24 = local_18 + local_c;
    local_1c = local_10 + local_4;
    if ((float *)&stack0xffffffc4 != &local_24) {
      fVar5 = local_24;
      fVar6 = local_20;
      _param_2 = local_1c;
    }
    local_10 = local_1c;
    local_c = local_24;
    local_4 = local_1c;
    Ball_SetTargetPos(param_1,fVar5,fVar6,_param_2);
    if (&local_24 != (float *)((int)param_1 + 0x76c)) {
      local_24 = *(float *)((int)param_1 + 0x76c);
      local_20 = *(float *)((int)param_1 + 0x770);
      local_1c = *(float *)((int)param_1 + 0x774);
    }
  }
  if (*(int *)((int)param_1 + 0x744) != 0) {
    iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,-0x32,'\x01');
    iVar3 = RNG_Rand(&PTR_OBJ_VTABLE,-0x32,'\x01');
    local_c = (float)iVar3;
    local_8 = (float)iVar2;
    iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,-0x32,'\x01');
    local_24 = local_c + local_24;
    local_20 = local_8 + local_20;
    local_1c = (float)iVar2 + local_1c;
  }
  if (*(int *)((int)this + 0x3f2c) != 0) {
    *(int *)((int)this + 0x3f2c) = *(int *)((int)this + 0x3f2c) + -1;
    local_24 = *(float *)((int)param_1 + 0x60);
    local_1c = *(float *)((int)param_1 + 0x68);
    local_20 = *(float *)((int)param_1 + 100);
    if ((float *)((int)param_1 + 0x76c) != &local_24) {
      *(float *)((int)param_1 + 0x76c) = local_24;
      *(float *)((int)param_1 + 0x770) = local_20;
      *(float *)((int)param_1 + 0x774) = local_1c;
    }
    if (pfVar1 != &local_24) {
      *pfVar1 = local_24;
      *(float *)((int)param_1 + 0x75c) = local_20;
      *(float *)((int)param_1 + 0x760) = local_1c;
    }
  }
  (**(code **)(**(int **)((int)this + 0x87c) + 8))();
  auVar4 = Wave_Cos(&PTR_PTR_004f7188,*(float *)((int)this + 0x29bc));
  local_4 = (float)(float10)auVar4;
  auVar4 = Wave_Sin(&PTR_PTR_004f7188,*(float *)((int)this + 0x29bc));
  local_18 = (float)(float10)auVar4;
  local_10 = local_4;
  local_14 = 0.9;
  (**(code **)(**(int **)((int)this + 0x87c) + 0xc))(&local_18,*(undefined4 *)((int)this + 0x29c0));
  (**(code **)(**(int **)(*(int *)((int)this + 0x878) + 0x174) + 4))
            (*(undefined4 *)((int)this + 0x87c));
  return;
}
