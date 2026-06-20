/*
 * Function: Ball_ApplyForceV2
 * Address: 0x004016F0
 * Signature: void __thiscall Ball_ApplyForceV2(void *this, float fx, float fy, float fz, float magnitude)
 *
 * Description:
 * Alternative force application method with gravity-plane awareness. This function
 * applies a directional force to the ball's velocity, with multipliers based on
 * game state. It also computes the ball's facing angle (rotation around Y or
 * gravity axis) using Math_Atan2Angle.
 *
 * Guards (all must be true):
 *   - ball+0x2F9 == 0 (not in tarpit/disabled)
 *   - ball+0x2CC == 0 (not frozen/stunned)
 *   - ball+0x808 == 0 (not in special mode, e.g. 8-ball)
 *   - ball+0x2F0 < 0x51 = 81 (frame counter must be < 81, i.e. during initial frames)
 *
 * Force multipliers applied to magnitude (param_4):
 *   - If ball+0x2F0 != 0 (not first frame): param_4 *= _DAT_004cf380
 *   - If ball+0x324 != 0 (on ice/special surface): param_4 *= _DAT_004cf378
 *   - If ball+0xC5C != 0 (in tube/pipe): param_4 *= _DAT_004cf374
 *     Also stores force direction at ball+0xFC/+0x100/+0x104 scaled by _DAT_004cf370
 *   - If ball+0xC4C != 0 (dizzy state): param_4 *= _DAT_004cf36c
 *
 * Velocity accumulation:
 *   ball+0x170 += fx * magnitude  (velocity X)
 *   ball+0x174 += fy * magnitude  (velocity Y)
 *   ball+0x178 += fz * magnitude  (velocity Z)
 *
 * Facing angle computation (if force is non-zero, i.e. not equal to _DAT_004cf368):
 *   Uses ball+0x748 as gravity plane selector:
 *     0: angle = atan2(fx, fz)   — Y-up gravity (standard)
 *     1: angle = atan2(-fy, fz)  — X-axis gravity (tilted/Up Race)
 *     2: angle = atan2(fx, fy)   — Z-axis gravity (flat/Odd Race)
 *   Result stored at ball+0x198, and ball+0x19C set to 1 (facing updated flag)
 *
 * Cross-references:
 *   - Referenced in vtables: 0x4CF32C, 0x4D5D3C, 0x4CF4AC, 0x4CF578 (Ball vtable entries)
 *   - This is the V2 variant — likely an improved version of an earlier ApplyForce
 *     that adds gravity-plane-aware facing angle computation
 *
 * Struct offsets used:
 *   ball+0x0FC: Tube force direction X (scaled)
 *   ball+0x100: Tube force direction Y (scaled)
 *   ball+0x104: Tube force direction Z (scaled)
 *   ball+0x170: Velocity X
 *   ball+0x174: Velocity Y
 *   ball+0x178: Velocity Z
 *   ball+0x198: Facing angle (radians)
 *   ball+0x19C: Facing updated flag (byte)
 *   ball+0x2CC: Frozen/stunned flag
 *   ball+0x2F0: Frame counter (for startup)
 *   ball+0x2F9: Tarpit/disabled flag
 *   ball+0x324: On ice flag
 *   ball+0x748: Gravity plane index (0=Y-up, 1=X-axis, 2=Z-axis)
 *   ball+0x808: Special mode (8-ball)
 *   ball+0xC4C: Dizzy state
 *   ball+0xC5C: In tube flag
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
Ball_ApplyForceV2(void *this,float param_1,float param_2,float param_3,float param_4)

{
  float10 fVar1;
  
  if ((((*(char *)((int)this + 0x2f9) == '\0') && (*(char *)((int)this + 0x2cc) == '\0')) &&
      (*(int *)((int)this + 0x808) == 0)) && (*(int *)((int)this + 0x2f0) < 0x51)) {
    if (*(int *)((int)this + 0x2f0) != 0) {
      param_4 = param_4 * _DAT_004cf380;
    }
    if (*(char *)((int)this + 0x324) != '\0') {
      param_4 = param_4 * (float)_DAT_004cf378;
    }
    if (*(int *)((int)this + 0xc5c) != 0) {
      param_4 = param_4 * _DAT_004cf374;
      *(float *)((int)this + 0xfc) = param_1 * _DAT_004cf370;
      *(float *)((int)this + 0x100) = param_2 * _DAT_004cf370;
      *(float *)((int)this + 0x104) = param_3 * _DAT_004cf370;
    }
    if (*(char *)((int)this + 0xc4c) != '\0') {
      param_4 = param_4 * _DAT_004cf36c;
    }
    *(float *)((int)this + 0x170) = param_1 * param_4 + *(float *)((int)this + 0x170);
    *(float *)((int)this + 0x174) = param_2 * param_4 + *(float *)((int)this + 0x174);
    *(float *)((int)this + 0x178) = param_3 * param_4 + *(float *)((int)this + 0x178);
    if (((param_1 != _DAT_004cf368) || (param_2 != _DAT_004cf368)) || (param_3 != _DAT_004cf368)) {
      if (*(int *)((int)this + 0x748) == 0) {
        fVar1 = Math_Atan2Angle(&PTR_PTR_004f7188,param_1,param_3,0.0,0.0);
        *(float *)((int)this + 0x198) = (float)fVar1;
      }
      if (*(int *)((int)this + 0x748) == 1) {
        fVar1 = Math_Atan2Angle(&PTR_PTR_004f7188,-param_2,param_3,0.0,0.0);
        *(float *)((int)this + 0x198) = (float)fVar1;
      }
      if (*(int *)((int)this + 0x748) == 2) {
        fVar1 = Math_Atan2Angle(&PTR_PTR_004f7188,param_1,param_2,0.0,0.0);
        *(float *)((int)this + 0x198) = (float)fVar1;
      }
      *(undefined1 *)((int)this + 0x19c) = 1;
    }
  }
  return;
}
