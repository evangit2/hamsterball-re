/*
 * Function: Vec3_AddInPlace
 * Address: 0x004018F0
 * Signature: void __thiscall Vec3_AddInPlace(void *this, float *param_1)
 *
 * Description:
 * Adds param_1 to this in-place: this += param_1.
 * Each component (X, Y, Z) of the input vector is added to the corresponding
 * component of this vector.
 *
 * Layout:
 *   +0x00: X (float)
 *   +0x04: Y (float)
 *   +0x08: Z (float)
 *
 * Result: this = this + param_1
 *
 * Cross-references:
 *   - Called from FollowBall_Update (0x43ECC0) at three locations (0x43FD08,
 *     0x43FD91, 0x43FE1A) — FollowBall accumulates velocity/position updates
 *     by adding vectors in-place. This is likely used for camera tracking
 *     or AI ball movement accumulation.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_AddInPlace(void *this,float *param_1)

{
  *(float *)this = *param_1 + *(float *)this;
  *(float *)((int)this + 4) = param_1[1] + *(float *)((int)this + 4);
  *(float *)((int)this + 8) = param_1[2] + *(float *)((int)this + 8);
  return;
}
