/*
 * Function: RaceResults_Update
 * Address: 0x0044c7d0
 * Signature: void __fastcall ...(int param_1)
 * Parameters:
 *   param_1: RaceResults* this
 *
 * Description:
 * Updates race results display. Handles key input (Input_CheckKeyCombo→+0x20 flag). Decrements frame counter. When all frames processed: sets App+0x5FC=5 and App+0x69C=5 (results state). Increments frame counter at +0x10 each iteration. Speed multiplier at +0x20 controls iterations per call (×10).
 *
 * Struct offsets:
 *   +0x04 (parent), +0x0C (App→+0x5FC/+0x69C), +0x10 (frame count), +0x14 (max frames), +0x19 (active flag), +0x1C (width), +0x20 (speed mult)
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D6CBC
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
