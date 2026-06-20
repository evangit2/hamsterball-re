/*
 * Function: Arena_HandleCollision
 * Address: 0x0040e6a0
 *
 * Description:
 *
Arena collision event handler — processes arena collision events between game objects.
Dispatches based on event name string at ball+0x864:
  - E:CALLHAMMER → CreateBonkPopup (if difficulty≠0)
  - E:HAMMERCHASE → Hammer_ChaseStart
  - E:ALERTSAW1/2 → Saw_AlertActivate (for saw1/saw2 at this+0x4370/+0x4374)
  - E:ACTIVATESAW1/2 → Saw_Activate
  - E:ALERTJUDGES → iterates judge list (this+0x4BBC), calls Judge_Reset on each
  - E:BONKBASH / E:BONKPOPUP → various bonk/popup effects
  - E:PEGS / E:NOPEGS → peg counter management
  - E:HEATON/HEATOFF → Pendulum_AddIndex/removeIndex for heat-based collision
  - E:LIMIT → Gfx_SetRenderState (limit collision visual)
  - E:TRAPPOP → Rotator_StartSound

All events only fire if difficulty != 0 (checked via App+0x23C).
Cross-refs: 0 calls, 1 data ref (vtable entry — called from arena collision dispatch).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
