/*
 * Function: NeonCollisionEvents
 * Address: 0x00410d00
 *
 * Description:
 *
Collision event handler for limit objects (like DispatchCollisionEvents but with additional events).
Processes events at ball+0x864:
  - E:PEGS → increments peg counter at this+0x47F4, sets flag param_1+0x1E2=1
  - E:NOPEGS → decrements peg counter, sets flag param_1+0x1E3=1
  - E:TRAPPOP → Rotator_StartSound (if difficulty≠0)
  - E:HEATON → Pendulum_AddIndex (if difficulty≠0)
  - E:HEATOFF → Gfx_SetRenderState (remove from pendulum)
  - E:LIMIT → Gfx_SetRenderState (limit collision visual)
  Then calls DispatchCollisionEvents for standard event processing.

Cross-refs: 0 calls, 1 data ref (vtable entry).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
