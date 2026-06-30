/*
 * Function: Ball_SetTiltedGravity
 * Address: 0x00403100
 * Signature: void __fastcall Ball_SetTiltedGravity(int param_1)
 *
 * Description:
 * Sets the ball's gravity to "tilted" mode (X-axis gravity, for Up Race / Level 6).
 * This is structurally identical to Ball_ResetCollisionMesh, but with different
 * gravity values:
 *
 *   1. Sets gravity plane index (ball+0x748) = 1 (tilted/X-axis mode)
 *   2. Sets gravity direction at CollisionMesh+0xC8C/+0xC90/+0xC94:
 *      X = -1.0 (0xBF800000) — gravity pulls in -X direction
 *      Y = 0.0
 *      Z = 0.0
 *   3. Calls vtable[1] (Ball_InitPhysicsDefaults) to reinitialize physics
 *   4. Syncs ball+0xF8 = ball+0x748 (active gravity plane)
 *
 * Cross-references (3 call sites):
 *   - 0x409270: likely in Ball_Update or init path for Up Race
 *   - 0x40EE08: Scene_SetupLevel6 (Up Race level)
 *   - 0x414E68: ArenaLevel_Up_Init (Up Race arena)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_SetTiltedGravity(int param_1)

{
  int iVar1;
  undefined4 local_c [3];
  
  iVar1 = *(int *)(param_1 + 0x1a4);
  *(undefined4 *)(param_1 + 0x748) = 1;
  if ((undefined4 *)(iVar1 + 0xc8c) != local_c) {
    *(undefined4 *)(iVar1 + 0xc8c) = 0xbf800000;
    *(undefined4 *)(iVar1 + 0xc90) = 0;
    *(undefined4 *)(iVar1 + 0xc94) = 0;
  }
  (**(code **)(*(int *)(param_1 + 0x1c) + 4))();
  *(undefined4 *)(param_1 + 0xf8) = *(undefined4 *)(param_1 + 0x748);
  return;
}
