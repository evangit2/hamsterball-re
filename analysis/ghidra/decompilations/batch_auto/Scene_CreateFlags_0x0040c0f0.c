/*
 * Function: Scene_CreateFlags
 * Address: 0x0040c0f0
 *
 * Description:
 *
Factory function that creates FLAG and SMALLFLAG water ripple objects from
MeshWorld data. Iterates the MW object list, matching "FLAG" (4 chars) and
"SMALLFLAG" (9 chars) name prefixes. For each match:
  1. If no WaterRipple exists yet (Scene+0x3F18 == 0): creates one (0x8C bytes)
     via WaterRipple_Ctor with the scene's render context
  2. Appends the MW data to the ripple list at Scene+0x2160
  3. For SMALLFLAG: scales the position data by _DAT_004CF3F0 (making them smaller)

Cross-refs: 1 call — Scene_SpawnBallsAndObjects.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
