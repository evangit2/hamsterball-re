/*
 * Function: Ball_Shatter
 * Address: 0x00408D70
 * Signature: void __thiscall Ball_Shatter(void *this, int param_1)
 *
 * Description:
 * Splits the ball into three smaller balls — the signature move of the 8-ball
 * power-up. The original ball is consumed and three new split balls are created
 * with trajectories diverging from the original position.
 *
 * Logic:
 *   1. Guards: only executes if ball+0x324 == 0 (not on ice/special surface)
 *   2. Frees the ball's name string at +0xC28 if present
 *   3. Calls board vtable[30] (offset 0x78) — likely plays a split sound/effect
 *   4. Sets event flag +0x2E8 = 1 (split event triggered)
 *   5. If proximity flag +0x744 == 0 (no nearby target):
 *      a. If split counter +0xC50 < 1.0: selects sound based on fall state
 *         - If not falling (+0xC4C == 0): uses Scene+0x44C sound
 *         - If falling: uses Scene+0x450 sound
 *      b. Otherwise: uses Scene+0x500 sound
 *      c. Plays the selected sound
 *   6. Calls Scene_ForEachBall_SetVelocity to push nearby balls
 *   7. Loops 3 times (creates 3 split balls):
 *      a. Allocates 0xC64 bytes for each Ball_Split
 *      b. Calls Ball_Split_ctor (inherits physics, computes speed from parent radius)
 *      c. Sets +0xC9*4 = 1 (is_split flag)
 *      d. Calls vtable[1] (Ball_InitPhysicsDefaults) for each split ball
 *      e. Copies trajectory from parent CollisionMesh+0xCA4 to child
 *      f. Sets position: either from parent position or from param_1 array
 *         (allows custom spawn positions for each split ball)
 *      g. Calls Ball_SetTrajectory with parent's stored trajectory
 *      h. Sets player index = -1 (split balls are AI-controlled)
 *      i. Sets spin timers (+0x278, +0x27C) for visual effect
 *      j. Appends to ball list via AthenaList_Append
 *   8. Creates trail particles for visual effect
 *
 * Cross-references (2 call sites, both from FollowBall_Update):
 *   - 0x43FE36: FollowBall_Update — AI triggered split
 *   - 0x43F722: FollowBall_Update — another AI split trigger
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* ~3953 chars decompiled — see GhidraMCP for full body */
