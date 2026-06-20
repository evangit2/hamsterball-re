/*
 * Function: CollisionMesh_AddTriangle
 * Address: 0x00456120
 * Signature: void __thiscall ...(void *this, int param_1)
 * Parameters:
 *   this: CollisionMesh* | param_1: int (triangle node to add)
 *
 * Description:
 * Adds a triangle to the collision mesh. Appends to AthenaList at +0x430. Sets triangle's owner to this (param_1+8=this). 2 refs.
 *
 * Struct offsets:
 *   +0x430 (AthenaList of triangles), triangle+8 (owner=CollisionMesh)
 *
 * Cross-references:
 *   2 refs: Mesh_FindClosestCollision, vtable DATA
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
