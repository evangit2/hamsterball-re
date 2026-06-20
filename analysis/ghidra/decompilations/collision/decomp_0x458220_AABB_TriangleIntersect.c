// Function: AABB_TriangleIntersect
// Address: 0x00458220
// Calling Convention: __cdecl
// Parameters: float *param_1 (Vec3 vertex A), float *param_2 (Vec3 vertex B)
// Returns: uint32 (bit 0 = intersection result, bits 8+ = FPU status flags)
// Xrefs: Called 6× from AABB_TriangleTest6Edges (0x00458364, 0x00458382, 0x00458394, 0x004583ae, 0x004583c4, 0x004583d6)
//
// PURPOSE: Tests whether a triangle edge (defined by two vertices param_1 and param_2)
// overlaps an AABB. The third test vertex is carried in FPU register state (unaff_ESI,
// fStack_24, fStack_20 — set by the caller before calling).
//
// ALGORITHM:
// 1. Compute per-axis min/max from param_1 and param_2 (forms the edge's bounding box)
// 2. Test if the third point (in FPU registers) is within the AABB on all 3 axes (X, Y, Z)
// 3. Return 1 if all axes pass (intersection), 0 otherwise
//
// The 6 calls from AABB_TriangleTest6Edges correspond to:
// - 3 calls testing each triangle vertex against the AABB
// - 3 calls testing each AABB corner against the triangle's edge intervals
// Together they form a complete Separating Axis Theorem (SAT) test.

undefined4 AABB_TriangleIntersect(float *param_1, float *param_2)
{
  // ... decompiled body ...
  // Computes min/max per axis from param_1 vs param_2
  // Tests third point (FPU regs) against all 3 axis ranges
  // Returns 1 if point is inside the AABB, 0 otherwise
}
