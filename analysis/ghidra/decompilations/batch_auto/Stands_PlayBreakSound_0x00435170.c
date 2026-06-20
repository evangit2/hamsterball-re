/*
 * Function: Stands_PlayBreakSound
 * Address: 0x00435170
 * Signature: void __fastcall Stands_PlayBreakSound(int param_1)
 * Parameters:
 *   param_1: Stands* this — the stands object that was hit/broken.
 *
 * Description:
 * Plays the break sound when a stands object is hit by a ball in the arena.
 * Only fires if the stands health (param_1+0x10E4) is exactly 3 (full health),
 * meaning the FIRST hit triggers the sound. On subsequent hits the health is
 * already decremented so this won't replay.
 *
 * When triggered:
 *   1. Sets health (param_1+0x10E4) = 0 (broken)
 *   2. Sets a timer/decay value at +0x10F8 = 0x3E800000 (float 0.25) — likely
 *      a 0.25-second delay before the stands disappears or starts falling.
 *   3. Plays a 3D positional sound via Sound_Play3D, using:
 *      - Sound buffer from App→Scene+0x10D0→+0x878→+0x47C (arena break sound)
 *      - Position from this+0x10D8 (X), +0x10DC (Y), +0x10E0 (Z)
 *
 * Struct offsets:
 *   param_1+0x10D0: parent Scene pointer (chain to App and sound system)
 *   param_1+0x10D8/+0x10DC/+0x10E0: stands position (X/Y/Z)
 *   param_1+0x10E4: health/state (3=intact, 0=broken)
 *   param_1+0x10F8: decay timer (0.25 seconds)
 *
 * Cross-references:
 *   Called from 0x40F074 — UNCONDITIONAL_CALL (from Arena_HandleCollision)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
