/*
 * Function: Ball_FindMeshCollision
 * Address: 0x00403980
 * Signature: undefined4 * __thiscall Ball_FindMeshCollision(void *this, undefined4 *param_1, ...)
 *
 * Description:
 * Wrapper function that prepares collision query parameters and delegates to
 * Mesh_FindClosestCollision. It copies two sets of 3-float vectors (likely
 * position and direction, or ray start and end points) from the input parameters
 * into local variables with self-assignment guards, then calls the mesh collision
 * function.
 *
 * The function takes 8 parameters (this + 7 params), where:
 *   param_1: output collision result pointer
 *   param_2,3,4: first Vec3 (likely ray origin or position)
 *   param_5,6,7: second Vec3 (likely ray direction or end point)
 *   param_8: additional parameter (radius? max distance?)
 *
 * Cross-references:
 *   - 0x403F6B: FollowBall_Update area — AI mesh collision queries
 *   - 0x440B5C: BounceBall_Update area — bounce collision detection
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

undefined4 * __thiscall
Ball_FindMeshCollision(void *this,undefined4 *param_1,undefined4 param_2,undefined4 param_3,
                      undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,
                      undefined4 param_8)
{
  /* ... see Ghidra decompilation ... */
}
