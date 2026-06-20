/*
 * Function: CreateMouseTrap
 * Address: 0x0040bf50
 *
 * Description:
 *
Factory function that creates MOUSETRAP game objects from MeshWorld data.
Iterates the MW object list, matching "MOUSETRAP" (exact stricmp). For each:
  1. Allocates 0x10F8 bytes for a TipperVisual_Level object
  2. Calls TipperVisual_Level_Ctor
  3. Appends to Scene+0xCD4, Scene+0x1930, collision lists, and render lists
  4. Copies position data from MW (3 floats) to +0x437/+0x438/+0x439
  5. Sets a Y-offset at +0x43D from _DAT_004CF44C - MW+0x14
  6. Calls vtable[11] (offset 0x2C) for post-initialization

Cross-refs: 1 call — Scene_SpawnBallsAndObjects.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
