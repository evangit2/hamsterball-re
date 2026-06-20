/*
 * Function: Lifter_PlaySound
 * Address: 0x00437300
 * Signature: void __fastcall Lifter_PlaySound(int param_1)
 * Parameters:
 *   param_1: Lifter* this — the lifter obstacle object to play sound for
 *
 * Description:
 * Plays the lifter activation sound, but only if it hasn't been played already
 * (checks +0x10E4 == 0). When triggered:
 *   1. Plays 3D positional sound via Sound_Play3D using:
 *      - Sound buffer from App→Scene+0x10D0→+0x878→+0x51C (lifter sound)
 *      - Position from this+0x10D8 (X), +0x10DC (Y), +0x10E0 (Z)
 *   2. Sets +0x10E4 = 1 (sound played flag = true)
 *   3. Sets +0x10E5 = 1 (triggered flag = true)
 *   4. Sets +0x10E8 = 300 (timer/effect duration, 5 seconds at 60fps)
 *
 * The lifter is a mechanical obstacle that pushes/launches the ball upward.
 * The sound plays once per activation, and the 300-frame timer controls the
 * duration of the lifting animation/effect.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer (chain to App and sound system)
 *   +0x10D8/+0x10DC/+0x10E0: position (X/Y/Z) for 3D sound
 *   +0x10E4: sound played flag (0→1)
 *   +0x10E5: triggered flag (0→1)
 *   +0x10E8: effect timer (300 frames = 5 seconds)
 *
 * Cross-references:
 *   Called from 0x416CD3 — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
