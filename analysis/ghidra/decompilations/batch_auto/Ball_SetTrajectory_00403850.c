/*
 * Function: Ball_SetTrajectory
 * Address: 0x00403850
 * Signature: void __thiscall Ball_SetTrajectory(void *this, undefined4 param_1, float x, float y, float z, float w)
 *
 * Description:
 * Sets the trajectory (launch direction and power) for the ball, typically used
 * by catapults, boost pads, and jump ramps. Stores both the raw direction and
 * a scaled version.
 *
 * Logic:
 *   1. Stores raw trajectory at ball+0x2AC/+0x2B0/+0x2B4/+0x2B8 (4 floats: x,y,z,w)
 *   2. Scales x,y,z by _DAT_004CF440 (trajectory power multiplier)
 *   3. Scales w by _DAT_004CF438 (separate w-scale, likely for vertical boost)
 *   4. Stores scaled trajectory at ball+0x1BC/+0x1C0/+0x1C4/+0x1C8
 *   5. Sets ball+0x204 = (scaled_w != _DAT_004CF3C8) — boolean flag: trajectory
 *      is "active" if the scaled w component differs from a threshold constant
 *   6. Copies scaled values to local Vec3 objects and resets matrices (SEH cleanup)
 *
 * The w component likely controls the vertical boost amount — when it matches
 * _DAT_004CF3C8 (possibly 0.0 or 1.0), the trajectory is considered inactive.
 *
 * Cross-references (7 call sites):
 *   - Board_ctor — setting default trajectories for level objects
 *   - Scene_SpawnBallsAndObjects — initial ball spawn trajectory
 *   - Ball_ctor2 area (0x40511F) — constructor initialization
 *   - CreateBadBall — enemy ball spawn trajectories
 *   - Ball_SplitIntoThree — split ball trajectories
 *   - Scene_SetupLevel6 — Up Race specific trajectories
 *
 * Struct offsets:
 *   ball+0x1BC/+0x1C0/+0x1C4/+0x1C8: Scaled trajectory (x,y,z,w)
 *   ball+0x204: Trajectory active flag (bool)
 *   ball+0x2AC/+0x2B0/+0x2B4/+0x2B8: Raw trajectory (x,y,z,w)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall
Ball_SetTrajectory(void *this,undefined4 param_1,float param_2,float param_3,float param_4,
                  float param_5)
{
  /* ... see Ghidra decompilation ... */
}
