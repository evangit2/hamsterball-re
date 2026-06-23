/*
 * Function: CheckArenaUnlock
 * Address: 0x0040aba0
 *
 * Description:
 *
Checks if an arena should be unlocked for the current level. Uses a switch
on the level type (App+0x220→+8) to select an arena unlock flag at different
App offsets (0x85A-0x868). If the flag is 0 (not yet unlocked):
  1. Plays unlock sound (App+0x518)
  2. Sets the flag to 1
  3. Copies camera profile from locked to unlocked
  4. Sets ball name to "ARENA UNLOCKED!"

Cross-refs: 1 call — DispatchCollisionEvents.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */


/* ~1324 chars — see GhidraMCP for full body */

