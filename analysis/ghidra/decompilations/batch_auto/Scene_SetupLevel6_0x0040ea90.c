/*
 * Function: Scene_SetupLevel6
 * Address: 0x0040ea90
 *
 * Description:
 *
Level setup for Tower Race (Level 5) — but stored at level6 slot. Creates:
  1. MeshWorld from "levels\level6" (0x10D0 bytes)
  2. CollisionLevel from the MeshWorld
  3. Calls Level_InitScene to initialize the scene
  4. Calls vtable[0x80] (offset 0x200) for post-init
  5. Looks up LAUNCH01, LAUNCH02, LAUNCH03 from AthenaHashTable — stores
     3-float positions at Scene+0x10DF/+0x10E2/+0x10E5 (launch pad positions)
  6. Looks up CHROMESHADOW — stores at Scene+0x10E9 (chrome shadow position)

Cross-refs: 0 calls, 1 data ref (vtable entry — called from Scene constructor).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
