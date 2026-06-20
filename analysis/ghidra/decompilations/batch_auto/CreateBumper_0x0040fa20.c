/*
 * Function: CreateBumper
 * Address: 0x0040fa20
 *
 * Description:
 *
Level setup for Expert Race (Level 8) with bumper objects. Creates:
  1. MeshWorld from "levels\level8" (0x10D0 bytes)
  2. CollisionLevel from the MeshWorld
  3. Calls Level_InitScene
  4. Loops 8 times (i=1..8): calls Scene_CollectByNameFilter with "N:BUMPER%d"
     to collect all 8 bumper objects from the MeshWorld data, storing them at
     Scene+0x10E3+i*0x106 (8 bumpers spaced 0x418 bytes apart)
  5. Calls vtable[0x80] for post-init

The 8 bumpers are the pinball-style bumpers in the Expert Race level.
Cross-refs: 0 calls, 1 data ref (vtable entry).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
