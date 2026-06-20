// Function: Collision_TraverseSpatialTree
// Address: 0x00465EF0
// Calling Convention: __thiscall (this = spatial tree node)
// Parameters: void *param_1 (query AABB center/point), void *param_2 (output AthenaList for matched faces)
// Xrefs: Called from Stands_BuildCollision (0x0043557C, 0x004355C9) and itself (recursive, 0x00465F3A)
//
// PURPOSE: Recursive octree traversal for broad-phase collision detection.
// Walks the spatial partitioning tree, and at leaf nodes, tests each collision
// face against the query AABB using AABB_ContainsPoint. Matching faces are
// appended to the output list.
//
// STRUCTURE:
// this+0x08 → parent collision object (has face lists at +0x2C/+0x30/+0x438)
// this+0x18 → child node list (AthenaList, count at +0x1C, data at +0x424)
// this+0x424 → child nodes array
//
// Parent's face storage:
// parent+0x2C → mesh buffer list (AthenaList)
// parent+0x30 → mesh buffer count
// parent+0x438 → mesh buffer data array
// Each mesh buffer: +0x0C = face list (AthenaList), +0x10 = face count, +0x418 = face data
// Each face: 8 floats (32 bytes) = 3 vertices × (pos+normal+uv) + padding? Actually
//   the loop reads 3 vertices per face (iVar5 = 3, do-while), each 8 floats apart.
//
// ALGORITHM:
// 1. Recurse into all child nodes (this+0x18 list)
// 2. At leaf level, iterate mesh buffers (parent+0x2C list)
// 3. For each mesh buffer, iterate faces (buffer+0x0C list)
// 4. For each face, test 3 vertices against query AABB via AABB_ContainsPoint
// 5. If any vertex is inside, append the face to param_2 output list

void __thiscall Collision_TraverseSpatialTree(void *this, void *param_1, void *param_2)
{
  // ... decompiled body ...
}
