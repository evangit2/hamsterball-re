/*
 * Function: Ball_SetFlatGravity
 * Address: 0x00403150
 * Signature: void __fastcall Ball_SetFlatGravity(int param_1)
 *
 * Description:
 * Sets the ball's gravity to "flat" mode (Z-axis gravity, for Odd Race / Level 9).
 * This is the third gravity variant (after standard Y-up and tilted X-axis):
 *
 *   1. Sets gravity plane index (ball+0x748) = 2 (flat/Z-axis mode)
 *   2. Sets gravity direction at CollisionMesh+0xC8C/+0xC90/+0xC94:
 *      X = 0.0
 *      Y = 0.0
 *      Z = 1.0 (0x3F800000) — gravity pulls in +Z direction
 *   3. Calls vtable[1] (Ball_InitPhysicsDefaults) to reinitialize physics
 *   4. Syncs ball+0xF8 = ball+0x748
 *
 * Cross-references (3 call sites):
 *   - 0x409280: likely in Ball_Update or init path for Odd Race
 *   - 0x40EE24: Scene_SetupLevel9 (Odd Race level)
 *   - 0x414E84: RumbleBoard_Odd_Init (Odd Race arena)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_SetFlatGravity(int param_1)

{
  int iVar1;
  undefined4 local_c [3];
  
  iVar1 = *(int *)(param_1 + 0x1a4);
  *(undefined4 *)(param_1 + 0x748) = 2;
  if ((undefined4 *)(iVar1 + 0xc8c) != local_c) {
    *(undefined4 *)(iVar1 + 0xc8c) = 0;
    *(undefined4 *)(iVar1 + 0xc90) = 0;
    *(undefined4 *)(iVar1 + 0xc94) = 0x3f800000;
  }
  (**(code **)(*(int *)(param_1 + 0x1c) + 4))();
  *(undefined4 *)(param_1 + 0xf8) = *(undefined4 *)(param_1 + 0x748);
  return;
}
