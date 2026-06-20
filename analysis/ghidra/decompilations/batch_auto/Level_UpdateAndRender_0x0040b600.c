/*
 * Function: Level_UpdateAndRender
 * Address: 0x0040b600
 *
 * Description:
 *
6-phase render update for the level scene:
1. Build visible_list: copies ball_list (Scene+0x29D4) and object_list (Scene+0x3204)
   into a temporary visible list (Scene+0x3A48)
2. Opaque pass: alpha OFF, calls ball->vtable[0x1C] (offset 0x70) for each ball
3. Alpha ON: enables alpha blending for transparent objects
4. Waypoint arrow: if race active and ball+0x70C flag set, renders directional arrow
   via ball+0x154→vtable[200] (0xC8 offset), increments counter at +0x7C8
5. Visible objects: calls vtable[8] (offset 0x20) for each visible object
6. Ball shadows: if water ripples exist, calls Ball_RenderShadow for each ball

Cross-refs: 2 calls, 30 data refs. Called from Scene_RenderWithCamera and 
scene render dispatch.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
