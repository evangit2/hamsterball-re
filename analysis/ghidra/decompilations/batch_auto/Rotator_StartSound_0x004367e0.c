/*
 * Function: Rotator_StartSound
 * Address: 0x004367E0
 * Signature: void __fastcall Rotator_StartSound(int param_1)
 * Parameters:
 *   param_1: Rotator* this — the rotator object to start sound for
 *
 * Description:
 * Starts the rotation sound for a Rotator obstacle, but only if it hasn't started
 * already (checks +0x10ED flag). When triggered:
 *   1. Sets +0x10ED = 1 (sound started = true)
 *   2. Resets +0x10E8 = 0 (timer/angle accumulator)
 *   3. Sets +0x10E4 = 0x3F800000 (float 1.0 — sound volume or rotation speed)
 *   4. Plays 3D positional sound via Sound_Play3D using:
 *      - Sound buffer from App→Scene+0x10D0→+0x878→+0x498 (rotator sound)
 *      - Position from this+0x10D4 (X), +0x10D8 (Y), +0x10DC (Z)
 *
 * The sound only plays once per rotator activation — the flag at +0x10ED prevents
 * replays on subsequent frames.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer (chain to App and sound system)
 *   +0x10D4/+0x10D8/+0x10DC: position (X/Y/Z) for 3D sound
 *   +0x10E4: sound volume/rotation speed (set to 1.0)
 *   +0x10E8: timer/angle accumulator (reset to 0)
 *   +0x10ED: sound started flag (0→1, prevents replay)
 *
 * Cross-references:
 *   Called from CreateLimit (0x410D73) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
