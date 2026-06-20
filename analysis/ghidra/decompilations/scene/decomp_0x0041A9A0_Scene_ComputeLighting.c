// Decompiled via GhidraMCP
// Function: Scene_ComputeInputForceDirection (Ghidra label: Scene_ComputeLighting — MISNOMER)
// Address: 0x0041A9A0
// Category: scene
// Vtable slot: 35 (+0x8C) of Board/Scene vtable
//
// PURPOSE: Computes a 3D directional force vector based on the strongest player input
// across all balls in a player slot. NOT a lighting function despite the Ghidra label.
//
// Algorithm:
//   1. Determine ball count for the given player slot (App+0xB28+slot*4), default 3
//   2. Iterate balls in slot (stride 4 starting at App+0x550):
//      - Call Ball_GetInputForce(ball, &force_x, &force_z) → reads keyboard/mouse/joystick
//      - Track ball with MAXIMUM input force magnitude (Vec2_Distance from origin)
//      - If winning ball's type (ball+8) == 2 (AI), set AI flag
//   3. If no input (max force == 0.0): fall back to default camera direction
//   4. Compute 3D direction: (input_dir - camera_pos), normalize via 1/sqrt(len^2)
//   5. Scale: 0.12 for human players; (App+0x84C * 89128.96 + 0) for AI balls
//   6. Output param_1 = normalized_direction * scale (Vec3)
//
// Key constants: _DAT_004cf368=0.0, _DAT_004cf310=1.0, _DAT_004d03b8=0.12 (human scale)
//
// Callers: RumbleBoard_Update (0x421FE0), Scene_ComputeArenaLighting (0x422C70),
//          Scene_ComputeArenaLighting2 (0x423800) — all via vtable dispatch

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