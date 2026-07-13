/*
 * Function: Vec3_NormalizeAndScale
 * Address: 0x00401AA0
 * Signature: void __thiscall Vec3_NormalizeAndScale(void *this, float param_1)
 *
 * Description:
 * Normalizes the vector (this) to unit length, then scales it by param_1.
 * In-place operation: this = (this / |this|) * param_1
 *
 * Logic:
 *   1. Computes length² = x² + y² + z²
 *   2. If length² is not negative and not zero:
 *      a. Computes length = SQRT(length²)
 *      b. If length > 0: computes scale = param_1 / length
 *      c. Multiplies each component by scale
 *   3. If length² is zero or negative, scale defaults to 0.0 (_DAT_004cf368),
 *      effectively zeroing the vector (safe fallback for degenerate inputs)
 *
 * This is one of the most widely-used utility functions in the engine — it
 * appears in 50+ call sites including ball physics, camera, collision, rendering,
 * particle systems, flag waves, and AI.
 *
 * Cross-references (most significant):
 *   - Ball_FallUpdate, Ball_AdvancePositionOrCollision (physics)
 *   - Scene_SetCamera (camera direction)
 *   - Ball_GetInputForce (input → force direction)
 *   - Ball_ApplyTrajectory (trajectory setup)
 *   - HandleArenaCollisionEvents, DispatchCollisionEvents (object creation)
 *   - TowerLevel_Ctor, CollisionFace_ctor (level construction)
 *   - FlagWaver_UpdateVertices (flag wave effects)
 *   - Mesh_FindClosestCollision (collision detection)
 *   - Font_DrawGlyph3D (3D text rendering)
 *   - FollowBall_Update (AI)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Vec3_NormalizeAndScale(void *this,float param_1)

{
  float fVar1;
  float fVar2;
  
  fVar1 = *(float *)((int)this + 8) * *(float *)((int)this + 8) +
          *(float *)((int)this + 4) * *(float *)((int)this + 4) + *(float *)this * *(float *)this;
  fVar2 = _DAT_004cf368;
  if ((fVar1 < _DAT_004cf368 == (fVar1 == _DAT_004cf368)) &&
     (fVar2 = SQRT(fVar1), _DAT_004cf368 < fVar2)) {
    fVar2 = param_1 / fVar2;
  }
  *(float *)this = fVar2 * *(float *)this;
  *(float *)((int)this + 4) = fVar2 * *(float *)((int)this + 4);
  *(float *)((int)this + 8) = fVar2 * *(float *)((int)this + 8);
  return;
}
