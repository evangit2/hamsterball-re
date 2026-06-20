/*
 * Function: TourneyMenu_TickWithRank
 * Address: 0x00451df0
 * Signature: void __fastcall ...(int *param_1)
 * Parameters:
 *   param_1: TourneyMenu* this (as int*)
 *
 * Description:
 * Per-frame tick with rank computation. Calls NoOp then __ftol2 for rank. Increments/decrements tournament score (+0x220). If score below threshold: calls vtable+0x40 (quit), CheckPurchaseOrHighScore. Counter at +0x32F increments, subtracts 0x4D per tick from +0x32C. 1 DATA ref.
 *
 * Struct offsets:
 *   +0x21E (App→+0x220), +0x21F (rank), +0x220 (score), +0x221 (direction flag), +0x32C (countdown), +0x32F (tick counter), +0x334 (max ticks), +0x335 (next state)
 *
 * Cross-references:
 *   1 DATA ref from vtable 0x4D862C
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
