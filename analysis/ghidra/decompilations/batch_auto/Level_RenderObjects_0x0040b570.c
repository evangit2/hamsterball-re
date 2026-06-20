/*
 * Function: Level_RenderObjects
 * Address: 0x0040b570
 *
 * Description:
 *
Transparent rendering pass for level objects:
1. Calls Graphics_BeginFrame(gfx, 0) to start rendering
2. Calls level mesh vtable[0x4C] (offset 0x130) to render the opaque mesh
3. Calls Graphics_BeginFrame again
4. Iterates the visible_object_list (AthenaList at Scene+0x3A48), calling 
   vtable[0xC] (offset 0x30, RenderTransparent) on each object

Cross-refs: 0 calls, 32 data refs — called via vtable from render pipeline.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
