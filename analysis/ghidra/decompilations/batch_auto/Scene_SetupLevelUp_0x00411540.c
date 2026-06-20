/*
 * Function: Scene_SetupLevelUp
 * Address: 0x00411540
 *
 * Description:
 *
Level setup for Up Race (Level 6). Creates:
  1. MeshWorld from "levels\levelup" (0x10D0 bytes)
  2. CollisionLevel from the MeshWorld
  3. Calls Level_InitScene and vtable[0x80]
  4. Sets default vacuum tube position at Scene+0x10D3 = (0, 50.0, 0)
  5. Sets camera distance at Scene+0xA70 = 900.0
  6. Iterates MW object list matching "VAC-IN" prefix:
     - For each VAC-IN object, finds corresponding "VAC-OUT" object
     - Creates vacuum tube pairs (suction tubes that blow the ball around)
     - Appends to vacuum tube list
  7. The vacuum tubes are the signature mechanic of Up Race

Cross-refs: 0 calls, 1 data ref (vtable entry).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
