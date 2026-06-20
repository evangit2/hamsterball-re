/*
 * Function: BounceBall_Update
 * Address: 0x00440840
 * Signature: undefined4 __fastcall BounceBall_Update(int *param_1)
 * Parameters:
 *   param_1: BounceBall* this (as int*) — the bounce ball to update
 *
 * Description:
 * Per-frame update for BounceBall objects — special balls that bounce and
 * eventually transform into FollowBall objects. Complex function:
 *   1. Reads scale at +0x439 and state at +0x43A (high 16 bits)
 *   2. If +0x43A == 0 (not yet transformed):
 *      a. Multiplies +0x439 by _DAT_004CF458 (scale shrink factor)
 *      b. If scale drops below _DAT_004D5DBC (minimum threshold):
 *         - Sets +0x439 = 120.0 (0x42F00000, reset to large size)
 *         - Sets +0x43A = 1 (mark as transformed)
 *         - Allocates FollowBall (0xC68 bytes) via operator_new
 *         - Calls FollowBall_Ctor with parent Scene (+0x434) and BallPath
 *         - Looks up "FOLLOWBALLSPOT" in hash table → stores spawn position
 *         - Sets FollowBall position at +0x164/+0x168/+0x16C
 *         - Sets CollisionMesh velocity at +0xCA4 = (-3.0, 10.0, 0)
 *         - Sets +0x80C = 0xF (ball type = 15, FollowBall)
 *         - Appends FollowBall to AthenaList
 *
 * The BounceBall shrinks over time. When it reaches minimum size, it transforms
 * into a FollowBall that follows a predefined path (BallPath found by name lookup).
 *
 * Struct offsets:
 *   +0x434: parent Scene pointer
 *   +0x439: scale (shrinks each frame)
 *   +0x43A: state flag (0 = shrinking, 1 = transformed)
 *   +0x80C: ball type (set to 15 = FollowBall)
 *   +0x164/+0x168/+0x16C: FollowBall spawn position (from FOLLOWBALLSPOT hash)
 *   +0x1A4: CollisionMesh pointer (sets velocity at +0xCA4/+0xCA8/+0xCAC)
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D5734 [DATA]
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
