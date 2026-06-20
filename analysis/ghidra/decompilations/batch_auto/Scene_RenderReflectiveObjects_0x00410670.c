/*
 * Function: Scene_RenderReflectiveObjects
 * Address: 0x00410670
 *
 * Description:
 *
Renders reflective objects (mirrors, chrome surfaces). Steps:
  1. Calls Level_RenderDynamicObjects to render sky and ripples
  2. Adjusts projection: adds _DAT_004CF308 to FOV values at ball+0x790/+0x794
  3. Calls Graphics_SetProjection with adjusted FOV
  4. Iterates 8 reflection sources at this+0x644C (float array):
     - For each non-zero reflection value, creates a RenderContext
     - Sets up reflection plane and scale (1.0)
     - Stores RenderContext at ball+0x7C0
     - Calls mesh vtable[0x50] (offset 0x140) with the reflection source
     - Resets matrix to identity after each
  5. Restores original projection

Cross-refs: 0 calls, 1 data ref (vtable entry — called from render pipeline).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
