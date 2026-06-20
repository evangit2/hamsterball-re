/*
 * Function: Ball_SetSpeed
 * Address: 0x004029C0
 * Signature: void __thiscall Ball_SetSpeed(void *this, float param_1)
 *
 * Description:
 * Sets the ball's speed scale and updates the gravity direction vector accordingly.
 *
 * Logic:
 *   1. Stores speed at ball+0xC64 (CollisionMesh speed_scale, overwritten
 *      every frame by Ball_Update)
 *   2. If gravity vector pointer (ball+0xC98) is valid (not a stack alias):
 *      a. ball+0xC98 = param_1 × ball+0xC8C  (gravity_dir.X × speed)
 *      b. ball+0xC9C = param_1 × ball+0xC90  (gravity_dir.Y × speed)
 *      c. ball+0xCA0 = param_1 × ball+0xC94  (gravity_dir.Z × speed)
 *
 * This scales the gravity direction vector by the new speed value, producing
 * a velocity vector. The base gravity direction is stored at ball+0xC8C/+0xC90/+0xC94
 * and the scaled velocity at ball+0xC98/+0xC9C/+0xCA0.
 *
 * Cross-references:
 *   - Ball_InitBattleMode (0x456CD0) — sets initial battle mode speed
 *   - Ball_Split_ctor (0x408D10) — when a split ball is created, inherits speed
 *
 * Struct offsets:
 *   ball+0xC64: speed_scale (set by param_1)
 *   ball+0xC8C/+0xC90/+0xC94: Gravity direction (base, unscaled)
 *   ball+0xC98/+0xC9C/+0xCA0: Gravity velocity (direction × speed)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_SetSpeed(void *this,float param_1)

{
  float local_c [3];
  
  *(float *)((int)this + 0xc64) = param_1;
  if ((float *)((int)this + 0xc98) != local_c) {
    *(float *)((int)this + 0xc98) = param_1 * *(float *)((int)this + 0xc8c);
    *(float *)((int)this + 0xc9c) = param_1 * *(float *)((int)this + 0xc90);
    *(float *)((int)this + 0xca0) = param_1 * *(float *)((int)this + 0xc94);
    return;
  }
  return;
}
