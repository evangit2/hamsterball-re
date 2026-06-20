/*
 * Function: Board_Beginner_HandleRaceEnd
 * Address: 0x00420240
 *
 * Description:
 *
Race end handler for Beginner Race. Calls Scene_HandleRaceEnd, then decays 4+
reflection values at +0x642C/+0x6430/+0x6434/+0x6438 by subtracting _DAT_004CF428
and clamping to minimum _DAT_004CF6A8. These are likely camera/mirror reflection
decay values that slowly fade after the race ends. 0 calls, 1 data ref (vtable).
~1651 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
