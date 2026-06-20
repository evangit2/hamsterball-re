/*
 * Function: Ball_AdvancePositionOrCollision
 * Address: 0x004564c0
 * Signature: void __thiscall ...(void *this, float *param_1, float *param_2, float *param_3, undefined4 param_4, float param_5)
 * Parameters:
 *   this: Ball* | param_1: float* (position delta) | param_2: float* (velocity) | param_3: float* (collision data) | param_4: undefined4 | param_5: float (dt)
 *
 * Description:
 * Core collision/physics advancement. If +0x14 flag set: iterates collision triangles, frees old data. Performs Material_Init, calls Ray_SetDirection for raycasting. This is the main function that moves the ball and checks collision with the level geometry. 2 refs.
 *
 * Struct offsets:
 *   +0x14 (flag), +0x18 (AthenaList of triangles), +0x1C (count), +0x424 (triangle array)
 *
 * Cross-references:
 *   2 refs: Mesh_FindClosestCollision, vtable DATA
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
