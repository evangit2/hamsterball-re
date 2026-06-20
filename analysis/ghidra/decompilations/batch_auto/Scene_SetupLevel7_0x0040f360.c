/*
 * Function: Scene_SetupLevel7
 * Address: 0x0040f360
 *
 * Description:
 *
Level setup for Neon Race (Level 7). Creates:
  1. MeshWorld from "levels\level7" (0x10D0 bytes)
  2. CollisionLevel from the MeshWorld
  3. Calls Level_InitScene
  4. Calls vtable[0x80] for post-init

Simple setup — no special object lookups or level-specific data.
Cross-refs: 0 calls, 1 data ref (vtable entry).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
