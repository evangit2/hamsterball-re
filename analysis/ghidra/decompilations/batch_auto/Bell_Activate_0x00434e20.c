/*
 * Function: Bell_Activate
 * Address: 0x00434E20
 * Signature: void __fastcall Bell_Activate(int param_1)
 * Parameters:
 *   param_1: int — pointer to the bell game object being activated.
 *     The bell object is accessed at offsets +0x10D0, +0x10E0, +0x10E4.
 *
 * Description:
 * Activates a bell object in the arena. When a ball collides with a bell
 * (dispatched from ExpertCollisionEvents at 0x40E9DB), this function:
 *   1. Plays the bell sound effect via Sound_PlayChannel, using the sound
 *      channel stored at App→Scene→bell_object+0x10D0→+0x878→+0x4C8.
 *      The chain: param_1+0x10D0 (parent scene) → +0x878 (App ptr) → +0x4C8 (sound channel ID).
 *   2. Sets param_1+0x10E4 = 0x42B40000 (float 90.0f) — likely the bell's
 *      activation timer or animation duration (90 frames or 1.5 seconds).
 *   3. Sets param_1+0x10E0 = 0x3F800000 (float 1.0f) — likely the bell's
 *      active state flag (1.0 = active) or animation progress reset.
 *
 * Struct offsets:
 *   param_1+0x10D0: parent Scene pointer (used to reach App and sound system)
 *   param_1+0x10E0: bell active state (float, set to 1.0)
 *   param_1+0x10E4: bell timer/duration (float, set to 90.0)
 *
 * Cross-references:
 *   Called from ExpertCollisionEvents (0x40E9DB) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* Full decompiled body:
void __fastcall Bell_Activate(int param_1)
{
  Sound_PlayChannel(*(int *)(*(int *)(*(int *)(param_1 + 0x10d0) + 0x878) + 0x4c8));
  *(undefined4 *)(param_1 + 0x10e4) = 0x42b40000;
  *(undefined4 *)(param_1 + 0x10e0) = 0x3f800000;
  return;
}
*/
