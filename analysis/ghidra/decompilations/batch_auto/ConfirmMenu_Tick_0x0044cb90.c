/*
 * Function: ConfirmMenu_Tick
 * Address: 0x0044cb90
 * Signature: void __fastcall ...(int param_1)
 * Parameters:
 *   param_1: ConfirmMenu* this
 *
 * Description:
 * Per-frame tick for ConfirmMenu. Handles key input (Input_CheckKeyCombo→+0x25 flag). Decrements timer at +0x20 by _DAT_004D6CB0. Increments frame counter at +0x10. When complete: sets App+0x5D6=1 (and another flag). Speed mult at +0x25 × 10 controls iterations.
 *
 * Struct offsets:
 *   +0x04 (parent), +0x0C (App→+0x5D6), +0x10 (frame count), +0x14 (max), +0x19 (active), +0x20 (timer), +0x24 (flag), +0x25 (speed mult)
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D6D00
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
