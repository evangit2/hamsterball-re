/*
 * Function: Scene_SetupLevel9
 * Address: 0x00410830
 *
 * Description:
 *
Level setup for Odd Race (Level 9). Creates:
  1. MeshWorld from "levels\level9" (0x10D0 bytes)
  2. CollisionLevel from the MeshWorld
  3. Calls Level_InitScene and vtable[0x80]
  4. Iterates MW object list looking for:
     - "PILLAR" (6 chars) → appends to Scene+0x10E5 (pillar list)
     - "MAGNIFYER" (9 chars) → if difficulty≠0, creates Magnifyer object
       (0x444 bytes) and appends to object+collision lists
  5. The magnifyer objects are the glass lens effects in Odd Race

Cross-refs: 0 calls, 1 data ref (vtable entry).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
