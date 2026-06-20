/*
 * Function: Scene_RenderReflectiveObjects7
 * Address: 0x00411380
 *
 * Description:
 *
Nearly identical to Scene_RenderReflectiveObjects but uses different struct offsets:
  - Reflection sources at this+0x642C (instead of +0x644C)
  - Reflection objects at this+0x436C (instead of +0x438C)

This variant is used for levels that store reflection data at different offsets
(likely Level 7 / Neon Race). Same 8-iteration loop with RenderContext setup
and vtable[0x50] mesh reflection rendering.

Cross-refs: 0 calls, 1 data ref (vtable entry).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
