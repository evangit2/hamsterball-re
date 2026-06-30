/*
 * Function: RaceResults_Reset
 * Address: 0x0044c450
 * Signature: void __fastcall ...(undefined4 *param_1)
 * Parameters:
 *   param_1: RaceResults* this
 *
 * Description:
 * Resets RaceResults. Sets vtable to PTR_RaceGoalReached_ScalarDtor (0x4D6CA0). Frees sub-object at +0x15 (0x54 offset) if non-NULL. Calls ToggleTimer_Cleanup on +0xF (0x3C offset). Sets vtable to 0x4D6C00. 1 call.
 *
 * Struct offsets:
 *   +0x00 (vtable), +0x15 (sub-object ptr→0x54), +0xF (timer→0x3C)
 *
 * Cross-references:
 *   1 call from RaceGoalReached_ScalarDtor (0x44D643)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
