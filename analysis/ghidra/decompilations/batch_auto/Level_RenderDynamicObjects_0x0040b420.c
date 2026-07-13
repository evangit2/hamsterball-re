/*
 * Function: Level_RenderDynamicObjects
 * Address: 0x0040b420
 *
 * Description:
 *
3-phase rendering pass for dynamic level objects:
1. Sky rendering: if Scene+0x3A44 is 0, calls skybox vtable[48] (0x48 offset); 
   otherwise calls skydome vtable[48] with params (1,1).
2. Water ripples: iterates the ripple list at Scene+0x2160. For each ripple:
   sets position, applies X/Z scaling (scaleXZ=_DAT_004CF44C-ripple+0x14, 
   scaleZ=-ripple+0x10), sets render position, calls FlagWaver_Render.
3. Dynamic objects: calls the dynamic_object_list vtable[8] (offset 0x20) 
   to render each dynamic object.

Cross-refs: 9 calls, 23 data refs (vtable entries). Called from Scene_RenderWithCamera,
Scene_RenderReflectiveObjects (2x), and other scene render passes.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
