/*
 * Function: Catapult_AddObjectConditional
 * Address: 0x0043E9C0
 * Signature: void __thiscall Catapult_AddObjectConditional(void *this, int param_1)
 * Parameters:
 *   this: Catapult* — the catapult object
 *   param_1: int — pointer to the object (Ball*) being added
 *
 * Description:
 * Adds an object to the catapult's tracking list, but only if a condition flag
 * at +0x1510 is set (non-zero). This is the conditional version of Stands_AddObject:
 *   1. Checks if +0x1510 != 0 (launch pad is active/enabled)
 *   2. If enabled, uses the same logic as Stands_AddObject:
 *      a. Gets next free index from AthenaList at +0x10F8
 *      b. If object already in list, refreshes timer to 10
 *      c. Otherwise allocates 8-byte node (ptr + timer=10) and appends
 *   3. If +0x1510 == 0, does nothing (catapult is disabled)
 *
 * The conditional check allows catapults to be activated/deactivated by game events
 * (e.g., only launch after a button is pressed or a timer expires).
 *
 * Struct offsets:
 *   +0x10F8: AthenaList (tracked objects)
 *   +0x10FC: list count
 *   +0x1100+: per-entry active flags
 *   +0x1504: list data pointer
 *   +0x1510: enable flag (0 = disabled, non-zero = enabled)
 *
 * Cross-references:
 *   Called from 0x4184D9 — UNCONDITIONAL_CALL (Scene_UpdateBallsAndState)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
