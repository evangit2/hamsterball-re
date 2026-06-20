/*
 * Function: Board_Up_HandleRaceEnd
 * Address: 0x00420660
 *
 * Description:
 *
Race end handler for Up Race. Calls Scene_HandleRaceEnd, then iterates the
AthenaList at +0x436C calling vtable[0] on each element (likely resetting
vacuum tubes or other Up Race specific objects). 0 calls, 1 data ref (vtable).
~803 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
