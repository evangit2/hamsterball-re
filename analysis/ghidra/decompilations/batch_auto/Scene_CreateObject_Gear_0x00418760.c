/*
 * Function: Scene_CreateObject_Gear
 * Address: 0x00418760
 *
 * Description:
 *
Factory for GEAR objects. Matches "GEAR" (4 chars) prefix. Allocates 0x1514
bytes, calls BigGear_ctor with position + rotation data from MW params.
Appends to Scene+0x2578 (rotator list). Returns object+type ID. Falls through
to CreatePlatformOrStands if no match. 1 vtable ref. ~1825 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
