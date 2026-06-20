/*
 * Function: Stands_BuildCollision
 * Address: 0x00435440
 * Signature: void __thiscall Stands_BuildCollision(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4)
 * Parameters:
 *   this: Stands* — the stands object to build collision data for
 *   param_1: undefined4 — stored at this+0x10EC (number of collision sections/slices)
 *   param_2: undefined4 — stored at this+0x10F4 (converted via __ftol2 and stored at +0x10F0)
 *   param_3: undefined4 — not directly stored (intermediate)
 *   param_4: undefined4 — stored at this+0x10DC (position or orientation override)
 *
 * Description:
 * Builds collision mesh data for Stands objects (breakable arena stands).
 * This is a complex function that:
 *   1. Stores collision section count at +0x10EC and a float→int conversion at +0x10F0
 *   2. Calls through this+0x480→+0x44C→vtable[0x14] to get vertex count for collision mesh
 *   3. Allocates vertexCount × 32 bytes (each vertex = pos+normal+uv, 32 bytes) via operator_new
 *      and stores the buffer at +0x10FC
 *   4. Calls vtable[0x38] to get the source vertex data pointer
 *   5. Copies vertex data from source to allocated buffer using a dword-copy loop
 *   6. Calls vtable[0x3C] to release the source data
 *   7. For each of the collision sections (count from +0x10EC):
 *      a. Initializes a material array
 *      b. Builds two AABBs (axis-aligned bounding boxes) — one for the upper
 *         half (y range: -10 to +10 around current y) and one for the lower half
 *         (y range: -20 to -30)
 *      c. Calls Collision_TraverseSpatialTree to find triangles in each AABB
 *         from the CollisionLevel at +0x10D4
 *      d. Calls MeshWorld_CollectTrianglesInAABB to gather triangle data
 *      e. Iterates through collected triangles and stores them
 *
 * This function essentially slices the stands mesh into horizontal bands and
 * builds collision data for each band — used for the breakable stand mechanic
 * where different sections can be hit independently.
 *
 * Struct offsets:
 *   +0x10D4: CollisionLevel pointer
 *   +0x10DC: position/orientation override
 *   +0x10EC: collision section count
 *   +0x10F0: converted float→int value
 *   +0x10F4: float parameter
 *   +0x10FC: vertex buffer pointer (allocated, vertexCount × 32 bytes)
 *   +0x1100: vertex count
 *
 * Cross-references:
 *   Called from 0x40F609 — UNCONDITIONAL_CALL (from Scene_SetupLevel7)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
