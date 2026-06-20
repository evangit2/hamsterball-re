/*
 * Function: RumbleBoard_Neon_Init
 * Address: 0x00416f40
 *
 * Description:
 *
Arena init for Neon Arena. MeshWorld "levels\arena-neon", CollisionLevel,
then iterates MW objects looking for "FLICKRING" objects to create flickering
ring obstacles (via RumbleBoard_Stands_ctor). Also collects "N:BUMPER%d"
objects. CameraLookAt, vtable[0x80]. 1 vtable ref. ~5000 chars.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
