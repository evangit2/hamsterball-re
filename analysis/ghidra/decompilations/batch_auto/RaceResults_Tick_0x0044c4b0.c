/*
 * Function: RaceResults_Tick
 * Address: 0x0044c4b0
 * Signature: void __fastcall ...(int param_1)
 * Parameters:
 *   param_1: RaceResults* this
 *
 * Description:
 * Per-frame tick for race results display. Increments frame counter at +0x10. If past max (+0x14) and active (+0x19), decrements counter at parent+0x8B4. Handles key input: Input_CheckKeyCombo sets +0x58/+0x59 flags. Calls ToggleTimer_Tick on +0x3C. Decrements timer at +0x28 by _DAT_004D6CB0.
 *
 * Struct offsets:
 *   +0x04 (parent), +0x0C (input handler), +0x10 (frame count), +0x14 (max frames), +0x19 (active flag), +0x28 (timer), +0x3C (RumbleBoard timer), +0x58/+0x59 (key flags), parent+0x8B4 (active count)
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D6CA4
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
