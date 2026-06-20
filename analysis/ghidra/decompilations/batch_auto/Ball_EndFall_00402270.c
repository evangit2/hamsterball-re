/*
 * Function: Ball_EndFall
 * Address: 0x00402270
 * Signature: void __fastcall Ball_EndFall(int param_1)
 *
 * Description:
 * Ends the ball's falling state and restores normal physics parameters.
 * This is the counterpart to Ball_StartFall (0x402200).
 *
 * Sets three values:
 *   1. ball+0xC4C = 0 (clears dizzy/falling flag — exits Ball_FallUpdate path)
 *   2. ball+0x284 = 0x41D00000 = 26.0f (restores ball radius/speed to 26,
 *      the normal gameplay value — Ball_StartFall had set it to 13.0f)
 *   3. ball+0x188 = 0x40A00000 = 5.0f (restores timer/scale to 5.0,
 *      Ball_StartFall had set it to 2.5f)
 *
 * Cross-references:
 *   - Called from Scene_SetupLevel6 (0x40EA90) at two locations (0x40F04E, 0x40F09A).
 *     The Up Race level calls this to reset balls after they fall and respawn.
 *
 * Struct offsets:
 *   ball+0x188: Timer/scale value (restored to 5.0f)
 *   ball+0x284: Ball radius/speed (restored to 26.0f)
 *   ball+0xC4C: Dizzy/falling flag (cleared to 0)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_EndFall(int param_1)

{
  *(undefined1 *)(param_1 + 0xc4c) = 0;
  *(undefined4 *)(param_1 + 0x284) = 0x41d00000;
  *(undefined4 *)(param_1 + 0x188) = 0x40a00000;
  return;
}
