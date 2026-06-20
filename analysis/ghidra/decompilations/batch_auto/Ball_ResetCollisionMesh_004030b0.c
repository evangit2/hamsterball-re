/*
 * Function: Ball_ResetCollisionMesh
 * Address: 0x004030B0
 * Signature: void __fastcall Ball_ResetCollisionMesh(int param_1)
 *
 * Description:
 * Resets the ball's collision mesh state to default gravity configuration.
 * This is called when a ball respawns or when the level needs to reset
 * collision parameters.
 *
 * Logic:
 *   1. Gets CollisionMesh pointer from ball+0x1A4
 *   2. Resets ball+0x748 = 0 (gravity plane index: 0 = standard Y-up gravity)
 *   3. If CollisionMesh+0xC8C is valid (not a stack alias):
 *      a. CollisionMesh+0xC8C = 0    (gravity_dir.X = 0)
 *      b. CollisionMesh+0xC90 = -1.0 (gravity_dir.Y = -1.0, i.e. downward)
 *      c. CollisionMesh+0xC94 = 0    (gravity_dir.Z = 0)
 *      This sets standard gravity: (0, -1, 0) — straight down.
 *   4. Calls a vtable function at ball+0x1C → vtable[1] (offset 4) — this is
 *      likely Ball_InitPhysicsDefaults, which reinitializes physics parameters
 *      like friction, max_speed, radius, etc.
 *   5. Copies ball+0x748 back to ball+0xF8 (syncs the gravity plane index
 *      to a second field, likely the "active" gravity plane used by rendering)
 *
 * Cross-references:
 *   - Ball_FindClosestRespawnPoint (0x405190) — called when ball needs to
 *     respawn, resets collision before repositioning
 *   - Scene_SetupLevel6 (0x40EA90) — Up Race level setup, resets collision
 *     for balls at the start
 *   - RumbleBoard_Odd_Init (0x414CE0) — Odd Race arena init
 *
 * Struct offsets:
 *   ball+0x1A4: CollisionMesh pointer (size 0xCB0)
 *   ball+0x1C:  vtable pointer (second vtable, likely Ball_Physics vtable)
 *   ball+0xF8:  Active gravity plane index (copy of +0x748)
 *   ball+0x748:  Gravity plane index (0=Y-up, 1=X-axis, 2=Z-axis)
 *   CollisionMesh+0xC8C/+0xC90/+0xC94: Gravity direction vector (reset to 0,-1,0)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_ResetCollisionMesh(int param_1)

{
  int iVar1;
  undefined4 local_c [3];
  
  iVar1 = *(int *)(param_1 + 0x1a4);
  *(undefined4 *)(param_1 + 0x748) = 0;
  if ((undefined4 *)(iVar1 + 0xc8c) != local_c) {
    *(undefined4 *)(iVar1 + 0xc8c) = 0;
    *(undefined4 *)(iVar1 + 0xc90) = 0xbf800000;
    *(undefined4 *)(iVar1 + 0xc94) = 0;
  }
  (**(code **)(*(int *)(param_1 + 0x1c) + 4))();
  *(undefined4 *)(param_1 + 0xf8) = *(undefined4 *)(param_1 + 0x748);
  return;
}
