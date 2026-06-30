/*
 * Function: CreateBadBall
 * Address: 0x0040bca0
 *
 * Description:
 *
Factory function that creates enemy "BadBall" objects from MeshWorld data.
Iterates the MW object list, matching "BADBALL" name. For each match:
  1. Allocates 0xC98 bytes for a Ball object, calls Ball_ctor
  2. Calls vtable[1] (Ball_InitPhysicsDefaults)
  3. Sets position at +0x164/+0x168/+0x16C from MW data + _DAT_004CF55C offset
  4. Clears unused_init_flag at +0x281 (DEAD: never read by any function)
  5. Stores raw position at +0xC60/+0xC64/+0xC68
  6. Parses sub-tags (CHASE, SPEED, etc.) from MW XML
  7. Appends to Scene+0xCD4 (object list) and Scene+0x29D4 (ball list)

Cross-refs: 1 call — Scene_SpawnBallsAndObjects.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
