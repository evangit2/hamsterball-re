/*
 * Function: CreateFlickRing
 * Address: 0x004173b0
 *
 * Description:
 *
Factory for FLICKRING objects. Matches "FLICKRING" (9 chars) prefix.
Allocates 0x1104 bytes, calls RumbleBoard_Stands_ctor with scene's spatial
tree (this+0x47E0). Appends to Scene+0x2578 (rotator list). Returns object
and type ID. Falls through to CreatePlatformOrStands if no match. 1 vtable ref.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
