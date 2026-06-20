/*
 * Function: Ball_ctor2
 * Address: 0x004039E0
 * Signature: Ball * __thiscall Ball_ctor2(void *this, int param_1)
 *
 * Description:
 * The primary Ball constructor. Constructs a Ball object (size 0xC98+ bytes)
 * with all default physics, rendering, and game state initialized.
 *
 * param_1 is the parent Board/Scene object.
 *
 * Initialization sequence (SEH-protected, 8 cleanup stages):
 *   1. Sets vtable to GameObject_sub2_dtor (0x4CF314) — base class vtable
 *   2. Constructs UITimer at +0x1C
 *   3. Initializes Timer at +0x108
 *   4. Initializes RenderContext at +0x1B8 and +0x208 (two render contexts)
 *   5. Initializes RumbleBoard timer at +0x264
 *   6. Initializes Vec3 at +0x2A8
 *   7. Initializes AthenaList at +0x32C (collision contacts) and +0x810 (trail particles)
 *   8. Stores board pointer at +0x14, sets player index to -1 (no player)
 *   9. Allocates CollisionMesh (0xCB0 bytes) via operator_new + CollisionMesh_ctor
 *   10. Sets default gravity (0,-1,0) and calls Ball_InitPhysicsDefaults via vtable[1]
 *   11. Initializes dozens of fields:
 *       - Position/velocity at +0x164/+0x168/+0x16C and +0x170/+0x174/+0x178 = 0
 *       - Max speed at +0x188 = 5000.0 (0x459C4000)
 *       - Speed scale at +0x18C = 1.0
 *       - Various flags and counters
 *       - Random value at +0x154 via RNG_Rand
 *       - Gravity scale at +0x278 = 0.1 (0x3DCCCCCD)
 *       - Teleport counter at +0xC38 = -1
 *
 * Cross-references (7 call sites):
 *   - Ball_ctor (0x40AFE0) — the public constructor wrapper
 *   - Board_ctor (2 calls) — constructing balls for the board
 *   - Scene_SpawnBallsAndObjects — spawning balls during gameplay
 *   - Ball_Split_ctor — constructing split balls
 *   - CreateBadBall — constructing enemy balls
 *   - GameObject_sub_ctor — constructing sub-class balls
 *
 * This is one of the most important functions in the engine — it defines
 * the complete Ball struct layout with 100+ field initializations.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* Ball_ctor2 decompilation is ~7400 chars — see GhidraMCP for full body */
