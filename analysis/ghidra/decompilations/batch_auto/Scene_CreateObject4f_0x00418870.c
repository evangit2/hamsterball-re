/*
 * Function: Scene_CreateObject4f
 * Address: 0x00418870
 *
 * Description:
 *
Renders a 4-float game object (quad). Calls Ball_GetTransform to get the
current transform, then Gfx_DrawQuadRandomColor to draw a randomly-colored
quad at the specified position. Resets matrix after. Called 25x — heavily used
for visual effects, particles, and debug rendering.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
