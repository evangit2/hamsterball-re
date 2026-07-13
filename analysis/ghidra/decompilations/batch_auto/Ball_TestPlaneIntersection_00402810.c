/*
 * Function: Ball_TestPlaneIntersection
 * Address: 0x00402810
 * Signature: short __thiscall Ball_TestPlaneIntersection(void *this, float a, float b, float c, float d)
 *
 * Description:
 * Tests whether the ball intersects with a plane. The plane is defined by the
 * equation: a*x + b*y + c*z + d = 0 (standard plane equation).
 *
 * Logic:
 *   1. For each of 6 points (iterating through ball+0x0C at stride 16 bytes):
 *      These 6 points are likely the ball's bounding box vertices or
 *      precomputed collision sphere sample points.
 *   2. Computes the signed distance from each point to the plane:
 *      distance = a*x + b*y + c*z + d
 *      (Note: the decompilation shows swapped variable names due to
 *       __thiscall parameter ordering)
 *   3. If any point's distance < -radius (fVar1 = -param_4, the negative
 *      ball radius or threshold): returns immediately with a "hit" result
 *   4. If all 6 points pass: returns 1 (no intersection, CONCAT11 with 1)
 *
 * The function handles NaN cases (via NAN() checks) for robustness.
 *
 * Cross-references (4 call sites):
 *   - Ball_Render (0x402DE0) — clipping test during rendering
 *   - FlagWaver_Render — testing if flag waves intersect with ball
 *   - SceneObject_ComputeCollisionSphere — collision sphere computation
 *   - 0x402C87 — likely another collision check
 *
 * Struct offsets:
 *   ball+0x0C: Array of 6 points (16 bytes each = 96 bytes total)
 *     Each point: X(float), Y(float), Z(float), W(padding/state)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

short __thiscall
Ball_TestPlaneIntersection(void *this,float param_1,float param_2,float param_3,float param_4)
{
  /* ... see Ghidra decompilation ... */
}
