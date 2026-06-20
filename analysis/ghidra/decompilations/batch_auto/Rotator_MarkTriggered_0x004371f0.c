/*
 * Function: Rotator_MarkTriggered
 * Address: 0x004371F0
 * Signature: void __fastcall Rotator_MarkTriggered(int param_1)
 * Parameters:
 *   param_1: Rotator* this — the rotator/obstacle object to mark as triggered
 *
 * Description:
 * Marks a rotator/obstacle as triggered by setting the flag at +0x10E4 = 1.
 * This is a simple one-line function — it sets the "triggered" flag which
 * other functions check to determine if the obstacle has been activated
 * by a ball collision or other event.
 *
 * Struct offsets:
 *   +0x10E4: triggered flag (set to 1)
 *
 * Cross-references:
 *   Called from CreateNoDizzy (0x40C617) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
