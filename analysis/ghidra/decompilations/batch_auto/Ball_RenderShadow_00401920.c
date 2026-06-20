/*
 * Function: Ball_RenderShadow
 * Address: 0x00401920
 * Signature: void __fastcall Ball_RenderShadow(int param_1)
 *
 * Description:
 * Renders the ball's shadow on the ground beneath it. The shadow is a flattened,
 * scaled version of the ball projected downward.
 *
 * Logic:
 *   1. Gets the graphics/scene device from: ball→Scene(+0x10)→+0x174→+0x154.
 *      Calls vtable[50] (offset 200/4) to set up the rendering state (likely
 *      disabling depth writes for the shadow pass).
 *   2. If ball+0x754 < 3 (shadow detail level check — 3 tiers of shadow quality):
 *      a. Initializes a Timer for the shadow render
 *      b. Computes shadow size = ball+0x284 (ball radius) * _DAT_004cf388 (scale constant)
 *      c. Sets the position to ball+0x164/+0x168/+0x16C (ball's X/Y/Z position)
 *      d. Copies 16 floats (64 bytes) from the scene object — likely a precomputed
 *         shadow matrix or transform
 *      e. Calls vtable[7] (offset 0x1C/4=7) on the shadow geometry object, indexed
 *         by ball+0x754 (the detail level selects which shadow mesh to render)
 *      f. Cleans up Timer
 *   3. Calls vtable[50] again with (0xA8, 0xF) to restore rendering state
 *      (re-enable depth writes, set blend mode, etc.)
 *
 * The shadow is only rendered for detail levels 0-2 (ball+0x754 < 3).
 * Level 3 presumably means "no shadow" (highest performance mode).
 *
 * Cross-references:
 *   - Called from Level_UpdateAndRender (0x40B600) at two points — likely once for
 *     the main render pass and once for a reflection/mirror pass
 *
 * Struct offsets:
 *   ball+0x10:  Scene pointer
 *   ball+0x164: Position X
 *   ball+0x168: Position Y
 *   ball+0x16C: Position Z
 *   ball+0x284: Ball radius (used for shadow size)
 *   ball+0x754: Shadow detail level (0-2 renders shadow, 3 = no shadow)
 *   scene→+0x174: Graphics/transform object
 *   graphics→+0x154: Render device
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_RenderShadow(int param_1)
{
  /* ... see decompiled body above ... */
}
