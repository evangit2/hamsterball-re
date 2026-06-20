/*
 * Function: Scene_CreateSigns
 * Address: 0x0040c270
 *
 * Description:
 *
Factory function that creates SIGN game objects from MeshWorld data.
Iterates the MW object list, matching "SIGN" (4 chars) name prefix.
For each match:
  1. Allocates 0x10FC bytes for a StandsTipper object
  2. Calls StandsTipper_Ctor with the scene's font+text resources
  3. Checks for "SIGN-TARPIT" suffix: if present, sets +0x10FC flag (tarpit sign)
  4. Appends to Scene+0xCD4, collision lists, and render lists

Cross-refs: 1 call — Scene_SpawnBallsAndObjects.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
