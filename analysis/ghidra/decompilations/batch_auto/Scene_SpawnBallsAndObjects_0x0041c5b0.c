/*
 * Function: Scene_SpawnBallsAndObjects
 * Address: 0x0041c5b0
 *
 * Description:
 *
Level startup — the master spawn function. Steps:
1. For each start entry: lookup "START%d-%d" in hash table for position,
   Ball_ctor2, set radius=26, max_speed=5, gravity=0.5
2. Scan SAFESPOT/SAFEPOS entries for safe spawn locations
3. If demo/tournament: CreateBadBall + CreateMouseTrap (enemy objects)
4. CreateFlags (water ripples), CreateSigns, CreateDynObjects
5. Calls each Create* factory in sequence: CreateSecretObjects, CreateBadBall,
   CreateMouseTrap, Scene_CreateFlags, Scene_CreateSigns, etc.

32 vtable data refs — called from every level constructor via vtable dispatch.
~5858 chars. 0 calls, 32 data refs.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
