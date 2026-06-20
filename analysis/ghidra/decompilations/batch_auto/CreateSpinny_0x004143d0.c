/*
 * Function: CreateSpinny
 * Address: 0x004143d0
 *
 * Description:
 *
Factory for SPINNY objects. Matches "SPINNY" (6 chars) name prefix. Allocates
0x1508 bytes, calls Rotator_ctor with the scene's spatial tree pointer
(this+0x47E0). Appends to Scene+0x2578 (rotator list). Returns the object and
its type ID via output params. If no match, falls through to CreatePlatformOrStands.
1 vtable ref.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
