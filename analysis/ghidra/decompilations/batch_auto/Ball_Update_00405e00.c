/*
 * Function: Ball_Update
 * Address: 0x00405E00
 * Signature: void __fastcall Ball_Update(int *param_1)
 *
 * Description:
 * THE MAIN BALL PHYSICS UPDATE — the largest and most important function in the
 * engine. This is called every frame for every ball to update its physics state.
 * 43,655 chars of decompiled code, ~1078 lines.
 *
 * 23-phase physics tick:
 *   Phase 1:  Decay timers (frame counter at +0x314, sound timer +0x317, etc.)
 *   Phase 2:  Play ambient sound (Sound_Play3DAtPosition if +0x315 active)
 *   Phase 3:  Spawn ambient trail particles (ArenaScoreParticle_ctor + AthenaList_Append)
 *   Phase 4:  Decay acceleration modifiers (+0x1D3, +0x1D4)
 *   Phase 5:  Decay counters (+0x1D1, +0xC0, +0xA8, +0x3)
 *   Phase 6:  Free display string if timer expired
 *   Phase 7:  Save previous position (prev_pos = pos at +0x158 = +0x164)
 *   Phase 8:  Build collision spatial tree (SpatialTree_ctor + CollisionNode_ctor)
 *   Phase 9:  Apply gravity (reflect velocity by gravity direction)
 *   Phase 10: Iterate collision list:
 *             type 1 = ball-ball collision (push apart, exchange velocity)
 *             type 2 = wall collision (reflect, apply friction)
 *             type 5 = floor collision (snap to surface, apply friction)
 *   Phase 11: Apply external velocity (boost trajectory at +0xCA4)
 *   Phase 12: Compute facing angle (atan2 based on gravity plane)
 *   Phase 13: Spin friction (3 iterations of velocity damping)
 *   Phase 14: Roll physics (surface alignment, angular velocity)
 *   Phase 15: Lerp display position toward actual position (smoothing)
 *   Phase 16: Teleport override (if teleport flag set, snap position)
 *   Phase 17: Overwrite CollisionMesh+0xC78 with ball radius (every frame)
 *   Phase 18: Apply trajectory boost (if +0x2F0 > 0, call Ball_ApplyTrajectory)
 *   Phase 19: Update sound position
 *   Phase 20: Update collision mesh state
 *   Phase 21: Check fall state (if +0xC4C, call Ball_FallUpdate)
 *   Phase 22: Check proximity events
 *   Phase 23: Finalize position and velocity
 *
 * Cross-references:
 *   - Called from 0x408300 and 0x4083BE (both in the Ball_Update dispatch area)
 *   - Referenced from Ball vtable at 0x4D5D28 (data ref)
 *
 * This function is the heart of the game's physics engine. It handles all
 * ball movement, collision response, gravity, friction, boost pads, falling,
 * and visual position smoothing.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* Ball_Update decompilation is ~43,655 chars — see GhidraMCP for full body.
 * Detailed phase analysis in decompilations/ball/decomp_ball_physics_update.c */
