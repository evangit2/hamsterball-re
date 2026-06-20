/*
 * Function: Ball_SetVec3AtOffset
 * Address: 0x00402A20
 * Signature: void __thiscall Ball_SetVec3AtOffset(void *this, undefined4 *param_1)
 *
 * Description:
 * Copies a 3-component vector (12 bytes: X, Y, Z) from param_1 into the ball
 * at offset +0xCA4/+0xCA8/+0xCAC. This is a guarded copy — only writes if the
 * source and destination pointers differ (prevents self-assignment).
 *
 * The offset +0xCA4 corresponds to the CollisionMesh's gravity direction vector
 * (grav_dir), which is also written by Ball_SetSpeed at +0xC8C/+0xC90/+0xC94.
 * However, +0xCA4 is a different field — likely the "target gravity" or
 * "external force" vector that gets blended with the base gravity.
 *
 * Cross-references (3 call sites):
 *   - Scene_ComputeLighting (0x41A9A0) — during scene lighting computation,
 *     likely setting a force direction for visual effects
 *   - BoardLevel9_Odd_dtor (0x41EE70) — during Odd Race level teardown
 *   - MeshNode_Level_DeleteDtor3 (0x437EF0) — during mesh node destruction
 *
 * Struct offsets:
 *   ball+0xCA4/+0xCA8/+0xCAC: 3-float vector (gravity/force direction)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_SetVec3AtOffset(void *this,undefined4 *param_1)

{
  if ((undefined4 *)((int)this + 0xca4) != param_1) {
    *(undefined4 *)((int)this + 0xca4) = *param_1;
    *(undefined4 *)((int)this + 0xca8) = param_1[1];
    *(undefined4 *)((int)this + 0xcac) = param_1[2];
  }
  return;
}
