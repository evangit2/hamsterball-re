/*
 * Function: Ball_StartFall
 * Address: 0x00402200
 * Signature: void __fastcall Ball_StartFall(int param_1)
 *
 * Description:
 * Initiates the ball's falling state. This is called when a ball goes off the
 * edge of the level and enters the "falling" animation/state.
 *
 * Sets three values on the ball:
 *   1. ball+0xC4C = 1 (dizzy/falling flag — enables Ball_FallUpdate path)
 *   2. ball+0x284 = 0x41500000 = 13.0f (sets the ball radius/speed value to 13,
 *      likely a smaller radius for fall animation or a fixed fall speed)
 *   3. ball+0x188 = 0x40200000 = 2.5f (sets a timer or scale value to 2.5,
 *      possibly the fall duration or animation speed)
 *
 * Then plays a 3D positioned sound effect:
 *   Sound_Play3D(scene→+0x4D4, ball+0x164, ball+0x168, ball+0x16C)
 *   The sound object is retrieved from Scene→+0x4D4, and the sound is played
 *   at the ball's current world position.
 *
 * Cross-references:
 *   - Called from Scene_SetupLevel6 (0x40EA90) — the Up Race level setup.
 *     This makes sense: the Up Race level has vertical sections where balls
 *     can fall off edges more frequently.
 *
 * Struct offsets:
 *   ball+0x10:  Scene pointer
 *   ball+0x164/+0x168/+0x16C: Ball position (X, Y, Z)
 *   ball+0x188: Timer/scale value (set to 2.5f)
 *   ball+0x284: Ball radius/speed (set to 13.0f)
 *   ball+0xC4C: Dizzy/falling flag (set to 1)
 *   scene+0x4D4: Fall sound object
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_StartFall(int param_1)

{
  *(undefined1 *)(param_1 + 0xc4c) = 1;
  *(undefined4 *)(param_1 + 0x284) = 0x41500000;
  *(undefined4 *)(param_1 + 0x188) = 0x40200000;
  Sound_Play3D(*(void **)(*(int *)(param_1 + 0x10) + 0x4d4),*(float *)(param_1 + 0x164),
               *(float *)(param_1 + 0x168),*(float *)(param_1 + 0x16c));
  return;
}
