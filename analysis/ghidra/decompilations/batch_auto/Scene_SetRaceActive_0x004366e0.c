/*
 * Function: Scene_SetRaceActive
 * Address: 0x004366E0
 * Signature: void __fastcall Scene_SetRaceActive(int param_1)
 * Parameters:
 *   param_1: int — pointer to the Scene/Board object
 *
 * Description:
 * Sets the race-active flag on a Scene object. Simply stores 1 at param_1+0x10EC
 * to mark the race as active (countdown finished, gameplay started). Called from
 * many different level constructors and race-start sequences.
 *
 * Struct offsets:
 *   +0x10EC: race_active flag (set to 1)
 *
 * Cross-references:
 *   Called from 10+ locations including BoardLevel constructors at 0x424334,
 *   0x4241BE, 0x4241CA, 0x4241D6, 0x4241E2, 0x4241EE, 0x4242A5, 0x41FD73, 0x41FD7E, etc.
 *   All UNCONDITIONAL_CALL.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
