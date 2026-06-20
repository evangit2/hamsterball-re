/*
 * Function: Trapdoor_Activate
 * Address: 0x00438410
 * Signature: void __fastcall Trapdoor_Activate(int param_1)
 * Parameters:
 *   param_1: Trapdoor* this — the trapdoor object to activate
 *
 * Description:
 * Activates a trapdoor (opens it so balls fall through). Conditions:
 *   1. Only activates if +0x10F4 == 0 (not already activated)
 *   2. Only plays sound if +0x10EC (current timer) < _DAT_004CF9F8 (a float threshold)
 *
 * When triggered:
 *   1. If timer is below threshold, plays 3D positional sound via Sound_Play3D
 *      using position at +0x10E0/+0x10E4/+0x10E8 and sound buffer from
 *      App→Scene+0x10D0→+0x878→+0x498 (trapdoor activation sound)
 *   2. Sets +0x10F4 = 10 (activation timer — 10 frames before trapdoor fully opens)
 *
 * The trapdoor is a level obstacle that opens when a ball touches a trigger,
 * causing the ball to fall through to a lower level or off the track entirely.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer (chain to App and sound system)
 *   +0x10E0/+0x10E4/+0x10E8: position (X/Y/Z) for 3D sound
 *   +0x10EC: current timer (compared against threshold)
 *   +0x10F4: activation state (0 = closed, 10 = opening)
 *
 * Cross-references:
 *   Called from Level_HandleCollision (0x40DE37) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
