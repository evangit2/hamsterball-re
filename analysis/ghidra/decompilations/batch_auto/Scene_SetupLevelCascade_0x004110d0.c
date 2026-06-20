/*
 * Function: Scene_SetupLevelCascade
 * Address: 0x004110d0
 *
 * Description:
 *
Level setup for Beginner Race (Level 2, "LevelCascade" in files). Creates:
  1. MeshWorld from "levels\levelcascade" (0x10D0 bytes)
  2. CollisionLevel from the MeshWorld
  3. Calls Level_InitScene and vtable[0x80]
  4. Loops 8 times: Scene_CollectByNameFilter "N:BUMPER%d" — collects 8 bumper
     objects at Scene+0x10DB+i*0x106 (same bumper layout as CreateBumper/Level8)

Despite the name "Cascade", this is actually Beginner Race (Level 2).
Cross-refs: 0 calls, 1 data ref (vtable entry).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
