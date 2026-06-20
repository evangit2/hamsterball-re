/*
 * Function: Ball_CheckProximity
 * Address: 0x00402150
 * Signature: void __thiscall Ball_CheckProximity(void *this, float x, float y, float z)
 *
 * Description:
 * Checks if a given point (x, y, z) is within proximity of the ball.
 * Computes the 3D distance from the point to the ball's position (ball+0x164/+0x168/+0x16C),
 * compares it against _DAT_004cf3d8 (a proximity threshold constant).
 *
 * If the distance is within the threshold:
 *   - Converts the boolean result to int (0 or 1) via __ftol2
 *   - Stores 1 at ball+0x744 (proximity flag)
 * If outside the threshold, ball+0x744 is NOT modified (retains previous value).
 *
 * This function is used to detect when balls are near each other or near
 * specific game objects.
 *
 * Cross-references:
 *   - Called from Scene_ForEachBall_SetVelocity (0x419B70) — iterates all balls
 *     to check proximity, likely for ball-ball interactions or collision avoidance
 *
 * Struct offsets:
 *   ball+0x164/+0x168/+0x16C: Ball position (X, Y, Z)
 *   ball+0x744: Proximity flag (set to 1 when point is within threshold)
 *
 * Constants:
 *   _DAT_004cf3d8: Proximity threshold distance
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_CheckProximity(void *this,float param_1,float param_2,float param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  ulonglong uVar4;
  
  fVar2 = param_1 - *(float *)((int)this + 0x164);
  fVar1 = param_2 - *(float *)((int)this + 0x168);
  fVar3 = param_3 - *(float *)((int)this + 0x16c);
  param_1 = (float)(uint)(SQRT(fVar2 * fVar2 + fVar1 * fVar1 + fVar3 * fVar3) < _DAT_004cf3d8);
  if ((float)(int)param_1 != _DAT_004cf368) {
    uVar4 = __ftol2();
    *(int *)((int)this + 0x744) = (int)uVar4;
  }
  return;
}
