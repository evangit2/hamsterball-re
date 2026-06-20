/*
 * Function: CreateLifter
 * Address: 0x00414a20
 *
 * Description:
 *
Factory for LIFTER objects. Matches "LIFTER" (6 chars) prefix, parses trailing
number via _atol (lifter ID). Allocates 0x10F4 bytes, calls Rotator_ctor_sound
with the lifter ID and scene's spatial tree (this+0x47E0). Appends to Scene+0x2578
(rotator list). Returns object+type ID. Falls through to CreatePlatformOrStands
if no match. 1 vtable ref.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
