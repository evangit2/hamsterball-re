/*
 * Function: ArenaCollisionLevel_Dtor
 * Address: 0x00437380
 * Signature: void __fastcall ArenaCollisionLevel_Dtor(undefined4 *param_1)
 * Parameters:
 *   param_1: RumbleCollision* this — the RumbleCollision level object to destroy
 *
 * Description:
 * Destructor for RumbleCollision level objects. Steps:
 *   1. Sets vtable to PTR_ArenaCollisionLevel_DeletingDtor (0x4D5A70)
 *   2. Calls ToggleTimer_Cleanup on +0x43B (cleans up a timer used for
 *      arena collision timing / respawn delays)
 *   3. Calls Level_Cleanup which frees CollisionLevel and other resources
 *
 * RumbleCollision is the collision system used in Arena mode (rumble boards).
 * It extends Level/Stands and includes a timer at +0x43B for managing the
 * knockoff/respawn cycle when balls collide in the arena.
 *
 * Struct offsets:
 *   +0x00: vtable (set to 0x4D5A70)
 *   +0x43B: RumbleBoard timer (cleaned up before Level_Cleanup)
 *
 * Cross-references:
 *   Called from ArenaCollisionLevel_DeletingDtor (0x43E5C3) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
