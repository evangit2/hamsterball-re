/*
 * Function: CollisionFace_ctor
 * Address: 0x00436860
 * Signature: void * __thiscall CollisionFace_ctor(void *this, undefined4 param_1, float param_2, float param_3, float param_4, float param_5, float param_6, float param_7, float param_8, float param_9, float param_10)
 * Parameters:
 *   this: CollisionFace* — the collision face object (size ~0x30 bytes)
 *   param_1: undefined4 — stored at this+0x04 (face ID or material index)
 *   param_2-4: float — stored at this+0x08/+0x0C/+0x10 (vertex 1: X, Y, Z)
 *   param_5-7: float — stored at this+0x14/+0x18/+0x1C (vertex 2: X, Y, Z)
 *   param_8-10: float — used to compute edge vectors for normal (v3.x, v3.y-v2.y, v3.z-v2.z)
 *
 * Description:
 * Constructor for CollisionFace objects — triangle collision faces used in
 * the spatial collision system. Steps:
 *   1. Stores face ID/material at +0x04
 *   2. Sets vtable to PTR_LAB_004D5768
 *   3. Stores vertex 1 at +0x08/+0x0C/+0x10 (X, Y, Z)
 *   4. Stores vertex 2 at +0x14/+0x18/+0x1C (X, Y, Z)
 *   5. Computes edge vectors: edge1 = v2 - v1, edge2 = v3 - v2
 *   6. Stores edge1 at +0x20/+0x24/+0x28 (edge vector components)
 *   7. Normalizes the edge vector using Vec3_NormalizeAndScale(this+0x20, 1.0)
 *      — this computes the face normal from the cross product of edges
 *   8. Sets +0x2C = 0 (flags or initialized to 0)
 *
 * CollisionFace is a key struct for collision detection — each face stores its
 * 3 vertices and a normalized normal vector used for plane equation testing.
 *
 * Struct offsets:
 *   +0x00: vtable (PTR_LAB_004D5768)
 *   +0x04: face ID/material
 *   +0x08/+0x0C/+0x10: vertex 1 (X, Y, Z)
 *   +0x14/+0x18/+0x1C: vertex 2 (X, Y, Z)
 *   +0x20/+0x24/+0x28: normalized face normal (X, Y, Z)
 *   +0x2C: flags (0)
 *
 * Cross-references:
 *   Called from Scene_SetupLevelUp (0x411736) — UNCONDITIONAL_CALL
 *   Used in BoardLevel_Up_Dtor (freed via CollisionFace_dtor_vtable)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
