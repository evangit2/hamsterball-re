/*
 * Function: Ball_ApplyTrajectory
 * Address: 0x00403750
 * Signature: void __fastcall Ball_ApplyTrajectory(int param_1)
 *
 * Description:
 * Applies a launch pad / boost trajectory to the ball. This is the function
 * called when a ball hits a catapult or boost pad in the race levels.
 *
 * Logic:
 *   1. Reads trajectory vector from CollisionMesh+0xCA4/+0xCA8/+0xCAC
 *   2. Sets ball+0x14D = 1 (boost/launch active flag)
 *   3. Normalizes the trajectory vector and scales by _DAT_004CF3F0 (boost strength)
 *   4. Damps the Y component of the original trajectory by _DAT_004CF434
 *      (reduces vertical boost to prevent flying too high)
 *   5. Plays boost sound via Sound_PlayChannel(Scene+0x4DC)
 *   6. Sets ball+0x2F0 = 100 (impact counter — blocks force application
 *      for ~100 frames / 1.67 seconds after launch)
 *   7. Calls Ball_CreateTrailParticles to spawn visual trail effect
 *   8. If ball+0x18 (player index) != -1: increments boost counter at
 *      App + playerIndex*0xA0 + 0x5F8 (tracks number of boosts per player)
 *
 * Cross-references:
 *   - Ball_Update (0x405E00) — called from the main physics update when
 *     a catapult/boost collision is detected
 *   - 0x410452 — another collision handler (likely in Scene_RenderReflectiveObjects
 *     or a level-specific collision handler)
 *
 * Struct offsets:
 *   ball+0x10:  Scene pointer
 *   ball+0x14D: Boost active flag (set to 1)
 *   ball+0x18:  Player index (-1 = no player)
 *   ball+0x2F0: Impact counter (set to 100, blocks force for 1.67s)
 *   CollisionMesh+0xCA4/+0xCA8/+0xCAC: Trajectory vector
 *   scene+0x4DC: Boost sound channel
 *   App+0x5F8+playerIndex*0xA0: Per-player boost counter
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_ApplyTrajectory(int param_1)
{
  /* ... see Ghidra decompilation ... */
}
